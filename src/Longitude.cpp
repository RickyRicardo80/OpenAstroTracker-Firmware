#include "../Configuration.hpp"
#include "Utility.hpp"
#include "Longitude.hpp"

//////////////////////////////////////////////////////////////////////////////////////
//
// -180..180 range, 0 is at the prime meridian (through Greenwich), negative going west, positive going east

Longitude::Longitude(const Longitude &other) : DayTime(other)
{
}

Longitude::Longitude(int h, int m, int s) : DayTime(h, m, s)
{
}

Longitude::Longitude(float inDegrees) : DayTime(inDegrees)
{
}

void Longitude::checkHours()
{
    while (totalSeconds > 180L * 3600L)
    {
        LOG(DEBUG_GENERAL, "[LONGITUDE]: CheckHours: Degrees is more than 180, wrapping");
        totalSeconds -= 360L * 3600L;
    }
    while (totalSeconds < (-180L * 3600L))
    {
        LOG(DEBUG_GENERAL, "[LONGITUDE]: CheckHours: Degrees is less than -180, wrapping");
        totalSeconds += 360L * 3600L;
    }
}

Longitude Longitude::ParseFromMeade(String const &s)
{
    Longitude result(0.0);
    LOG(DEBUG_GENERAL, "[LONGITUDE]: Parse(%s)", s.c_str());

    // Use the DayTime code to parse it.
    DayTime dt = DayTime::ParseFromMeade(s);

    if ((s[0] == '-') || (s[0] == '+'))
    {
        // According to spec 2010.10, if a sign is present it is straight up longitude with east as negative.
        result.totalSeconds = -dt.getTotalSeconds();
    }
    else
    {
        //from indilib driver:  Meade defines longitude as 0 to 360 WESTWARD (https://github.com/indilib/indi/blob/1b2f462b9c9b0f75629b635d77dc626b9d4b74a3/drivers/telescope/lx200driver.cpp#L1019)
        result.totalSeconds = 180L * 3600L - dt.getTotalSeconds();
    }
    result.checkHours();

    LOG(DEBUG_GENERAL, "[LONGITUDE]: Parse(%s) -> %s = %ls", s.c_str(), result.ToString(), result.getTotalSeconds());
    return result;
}

char achBufLong[32];

const char *Longitude::ToString() const
{
    long secs = totalSeconds;
    if (secs < 0)
    {
        secs += 360L * 3600L;
    }

    String totalDegs = String(1.0f * labs(totalSeconds) / 3600.0f, 2);
    String degs      = String(1.0f * secs / 3600.0f, 2);
    strcpy(achBufLong, degs.c_str());
    strcat(achBufLong, " (");
    strcat(achBufLong, totalDegs.c_str());
    strcat(achBufLong, (totalSeconds < 0) ? "W)" : "E)");
    return achBufLong;
}

const char *Longitude::formatString(char *targetBuffer, const char *format, long *) const
{
    long secs = labs(totalSeconds);

    long degs = secs / 3600;
    secs      = secs - degs * 3600;
    long mins = secs / 60;
    secs      = secs - mins * 60;
    if (totalSeconds < 0)
    {
        degs = -degs;
    }

    return formatStringImpl(targetBuffer, format, '\0', degs, mins, secs);
}

const char *Longitude::formatStringForMeade(char *targetBuffer) const
{
    LOG(DEBUG_MEADE, "[LONGITUDE] Format %l for Meade", totalSeconds);
    long secs = labs(totalSeconds);

    long degs = secs / 3600;
    secs      = secs - degs * 3600;
    long mins = secs / 60;
    secs      = secs - mins * 60;
    LOG(DEBUG_MEADE, "[LONGITUDE] Degs is %l, Mins is %l", degs, mins);

    // Since internal storage is actual longitude, Meade is negated
    if (totalSeconds > 0)
    {
        // Since we already inverted it when it was negative (by using ABS a few
        // lines above here), we only invert it if it is positive.
        degs = -degs;
    }

    LOG(DEBUG_MEADE, "[LONGITUDE] Inverted Degs, now %l, Mins is %l", degs, mins);

    return formatStringImpl(targetBuffer, "{+}{d}*{m}", '\0', degs, mins, secs);
}
