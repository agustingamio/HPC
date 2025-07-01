using HPCConcept.Structures;

namespace HPCConcept.Helper;

public static class DayTypeHelper
{
    public static DayType GetPreviousDayType(this DayType dayType)
    {
        return dayType switch
        {
            DayType.WorkingDay => DayType.Sunday,
            DayType.Saturday => DayType.WorkingDay,
            DayType.Sunday => DayType.Saturday,
            _ => throw new ArgumentOutOfRangeException(nameof(dayType), dayType, null)
        };
    }

    public static DayType GetDateType(this DateTime day)
    {
        return day.DayOfWeek switch
        {
            DayOfWeek.Saturday => DayType.Saturday,
            DayOfWeek.Sunday => DayType.Sunday,
            _ => DayType.WorkingDay
        };
    }
}