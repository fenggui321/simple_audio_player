
#ifndef _WIFIDOG_DEBUG_H_
#define _WIFIDOG_DEBUG_H_

#include <string.h>
#include <syslog.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)


/** Used to output messages.
 * The messages will include the filename and line number, and will be sent to syslog if so configured in the config file
 * @param level Debug level
 * @param format... sprintf like format string
 */

#define debug(level, format...) _debug(__FILENAME__, __LINE__, level, format)

/** @internal */
void _debug(const char *, int, int, const char *, ...);

void setDebug(int debuglevel);

#endif /* _DEBUG_H_ */
