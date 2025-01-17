/*
The MIT License (MIT)

Copyright (c) 2020 Lancaster University.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

/*
 * Log file system format:
 *
 * H = strlen(MICROBIT_LOG_HEADER) rounded up to page boundary.
 * N = (dataStart - H * PAGE_SIZE) / PAGE_SIZE
 * M = logLength - (N+H)*PAGE_SIZE
 * 
 * H x Header pages.
 * +===========================+
 * |     PREEFINED-HEADER      |
 * +===========================+
 * 
 * +===========================+
 * |                           |
 * |    MicroBitLogMetaData    |
 * |       (40 bytes)          |
 * |                           | 
 * +---------------------------+
 * |                           |
 * |     0x00 Init Region      |
 * |     (variable length)     |
 * |                           |
 * +---------------------------+
 * |                           |
 * | Comma separated Log Keys  |
 * |     (variable length)     |
 * |                           |
 * +---------------------------|
 * |                           |
 * |     0xFF Init Region      |
 * |     (variable length)     |
 * |                           |
 * +===========================+
 * 
 *
 * N x Journal Pages:
 * +===========================+
 * |                           |
 * |     0x00 Init Region      |
 * |     (variable length)     |
 * |                           |
 * +---------------------------+
 * |                           |
 * |      Logfile length       |
 * |         (12 bytes)        |
 * |                           |
 * +---------------------------|
 * |                           |
 * |     0xFF Init Region      |
 * |     (variable length)     |
 * |                           |
 * +===========================+
 * 
 * M x pages
 * +===========================+
 * |                           |
 * |         Log Data          |
 * |                           |
 * +---------------------------|
 * |                           |
 * |     0xFF Init Region      |
 * |     (variable length)     |
 * |                           |
 * +===========================+
 * 
 */

#ifndef MICROBIT_LOG_H
#define MICROBIT_LOG_H

#include "MicroBitUSBFlashManager.h"
#include "FSCache.h"
#include "ManagedString.h"

#ifndef CONFIG_MICROBIT_LOG_JOURNAL_PAGES
#define CONFIG_MICROBIT_LOG_JOURNAL_PAGES   2
#endif

#define MICROBIT_LOG_VERSION                "UBIT_LOG_FS_V_001\n"           // MUST be 18 characters.
#define MICROBIT_LOG_JOURNAL_ENTRY_SIZE     8

#define MICROBIT_LOG_STATUS_INITIALIZED     0x0001
#define MICROBIT_LOG_STATUS_ROW_STARTED     0x0002

namespace codal
{
    struct MicroBitLogMetaData
    {
        char        version[18];             // MICROBIT_LOG_VERSION
        char        logEnd[11];              // 32 bit HEX representation containing end address of available storage (e.g. "0x0000FFFF\n")
        char        dataStart[11];           // 32 bit HEX representation of logical start address of data file (e.g. "0x00000200\n")
    };

    class ColumnEntry
    {
        public:
        ManagedString key;
        ManagedString value;
    };

    
    enum class TimeStampFormat
    {
        None = 0,
        Milliseconds = 1,
        Seconds = 10,
        Minutes = 600,
        Hours = 36000,
        Days = 864000
    };

    struct JournalEntry
    {
        char        length[MICROBIT_LOG_JOURNAL_ENTRY_SIZE];
        char        null;

        JournalEntry()
        {
            memcpy(length, "00000000", MICROBIT_LOG_JOURNAL_ENTRY_SIZE);
            null = 0;
        }

        void clear()
        {
            memset(length, 0, MICROBIT_LOG_JOURNAL_ENTRY_SIZE);
        }

        bool containsOnly(uint8_t value)
        {
            for (int i=0; i<MICROBIT_LOG_JOURNAL_ENTRY_SIZE;i++)
                if (length[i] != value)
                    return false;

            return true;
        }
    };

    /**
     * Class definition for MicroBitLog. A simple text only, append only, single file log file system.
     * Also contains a key/value pair abstraction to enable dynamic creation of CSV based logfiles.
     */
    class MicroBitLog 
    {
        private:
        MicroBitUSBFlashManager         &flash;             // Non-volatile memory contorller to use for storage.
        FSCache                         cache;              // Write through RAM cache.
        uint32_t                        status;             // Status flags.
        FiberLock                       mutex;              // Mutual exclusion primitive to serialise APi calls.

        uint32_t                        startAddress;       // Logical address of the start of the Log file system.
        uint32_t                        journalPages;       // Number of physical pages allocated to journalling.
        uint32_t                        journalStart;       // logical address of the start of the journal section.
        uint32_t                        journalHead;        // Logical address of the last valid journal entry.
        uint32_t                        dataStart;          // Logical address of the start of the Data section.
        uint32_t                        dataEnd;            // Logical address of the end of valid data.
        uint32_t                        logEnd;             // Logical address of the end of the file system space.
        uint32_t                        headingStart;       // Logical address of the start of the column header data. Zero if no data is present.
        uint32_t                        headingLength;      // The length (in bytes) of the column header data.
        uint32_t                        headingCount;       // Total number of headings in the current log.
        bool                            headingsChanged;    // Flag to indicate if a row has been added that contains new columns.

        struct ColumnEntry*             rowData;            // Collection of key/value pairs. Used to accumulate each data row.
        struct MicroBitLogMetaData      metaData;           // Snapshot of the metadata held in flash storage.
        TimeStampFormat                 timeStampFormat;    // The format of timestamp to log on each row.
        ManagedString                   timeStampHeading;   // The title of the timestamp column, including units.

        const static uint8_t            header[2048];       // static header to prepend to FS in physical storage.

        public:

        /**
         * Constructor.
         */
        MicroBitLog(MicroBitUSBFlashManager &flash, int journalPages = CONFIG_MICROBIT_LOG_JOURNAL_PAGES);

        /**
         * Destructor.
         */
        ~MicroBitLog();

        /**
         * Reset all data stored in persistent storage.
         */
        void format();

        /**
         * Determines the format of the timestamp data to be added (if any).
         * If requested, time stamps will be automatically added to each row of data
         * as an integer value rounded down to the unit specified.
         * 
         * @param format The format of timestamp to use. 
         */
        void setTimeStamp(TimeStampFormat format);
         
        /**
         * Creates a new row in the log, ready to be populated by logData()
         * 
         * @return DEVICE_OK on success.
         */
        int beginRow();

        /**
         * Populates the current row with the given key/value pair.
         * @param key the name of the key column) to set.
         * @param value the value to insert 
         * 
         * @return DEVICE_OK on success.
         */
        int logData(const char *key, const char *value);

        /**
         * Populates the current row with the given key/value pair.
         * @param key the name of the key column) to set.
         * @param value the value to insert 
         *
         * @return DEVICE_OK on success.
         */
        int logData(ManagedString key, ManagedString value);

        /**
         * Complete a row in the log, and pushes to persistent storage.
         * @return DEVICE_OK on success.
         */
        int endRow();

        /**
         * Inject the given row into the log as text, ignoring key/value pairs.
         * @param s the string to inject.
         */
        int logString(const char *s);

        /**
         * Inject the given row into the log as text, ignoring key/value pairs.
         * @param s the string to inject.
         */
        int logString(ManagedString s);

    private:

        /**
         * Attempt to load an exisitng filesystem, or fomrat a new one if not found.
         */
        void init();

        /**
         * Add the given heading to the list of headings in use. If the heading already exists,
         * this method has no effect.
         * 
         * @param key the heading to add
         */
        void addHeading(ManagedString key, ManagedString value = ManagedString::EmptyString);

    };
}

#endif
