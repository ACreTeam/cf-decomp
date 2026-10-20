#pragma once

// Bulletin board notices. Source: src/dol/game/d_notice.cpp (.text 800EBB24..800ECD90).
// Automatic notice posting for the town board (see d_sv_notice for the board itself). The file
// name is a working name; function names are inferred.

#include <types.h>
#include <game/game/d_date.hpp>

class dNoticeData_c;

// ---------------------------------------------------------------------------
// Bulletin board posting (800EBB24..800ECD90).

namespace dNotice {

// Posts a BBS notice: message `msgId` from group `group` dated `date` (NULL = now).
void postNotice(u16 msgId, const char *group, const dTime_c *date, const void *sender); // 800EBB24
void addNotice(const dNoticeData_c *note); // 800EBCF4: add a note to the town board
void removeNotice(u8 idx); // 800EBD3C
dNoticeData_c *getNotice(int idx); // 800EBD78
int getNoticeCount(); // 800EBDB4: note count
BOOL hasUnreadNotice(); // 800EBDE0
void setNoticeRead(int idx); // 800EBEA4
BOOL isNoticeRead(int idx); // 800EBF14
void postDateNotices(const dTime_c &day); // 800EBF94: fixed-date notices
void postBirthdayNotices(const dTime_c &day); // 800EC060: birthday notices
void postEventNotices(const dTime_c &day); // 800EC19C: event notices
void postOctoberNotice(const dTime_c &day); // 800EC4B0
void postShopNotice(const dTime_c &day); // 800EC4F8: Tom Nook notice
void postWeatherNotice(const dTime_c &day); // 800EC72C
void postDailyNotices(const dTime_c &day); // 800EC77C: all notices for a day
void updateNotices(const dTime_c &now); // 800EC7C8: catch up since the last visit
void sortNewNotices(); // 800ECC78
void receiveAddNotice(int unused, const void *data); // 800ECCB4: net receive (0x19A bytes)
void sendAddNotice(const void *data); // 800ECD08: net send
void receiveRemoveNotice(int unused, const u8 *data); // 800ECD4C
void sendRemoveNotice(u8 idx); // 800ECD54

} // namespace dNotice
