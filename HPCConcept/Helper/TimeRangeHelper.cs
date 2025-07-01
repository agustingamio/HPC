using HPCConcept.Structures;

namespace HPCConcept.Helper;

public static class TimeRangeHelper
{
    public static TimeRange GetTimeRange( this TimeOnly time)
    {
        if (time >= new TimeOnly(0, 0) && time < new TimeOnly(6, 0))
            return TimeRange.EarlyMorning;
        if (time >= new TimeOnly(6, 0) && time < new TimeOnly(12, 0))
            return TimeRange.Morning;
        if (time >= new TimeOnly(12, 0) && time < new TimeOnly(18, 0))
            return TimeRange.Afternoon;
        if (time >= new TimeOnly(18, 0) && time < new TimeOnly(21, 0))
            return TimeRange.Evening;
        return TimeRange.Night;
    }
}