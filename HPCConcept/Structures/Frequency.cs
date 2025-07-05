using HPCConcept.Auxiliary;
using HPCConcept.Helper;

namespace HPCConcept.Structures;

public class Frequency
{
    public int VariantId { get; set; }
    public DayType DayType { get; set; }
    public TimeOnly DepartureTime { get; set; }
    public TimeSpan FrequencyAverage { get; set; }
    
    public static List<Frequency> CalculateFrequencyFromCsv(string filePath)
    {
        var departures = CsvReader.ReadPlannedRouteCsv(filePath)
            .Where(departure => departure.RelativeStopId == 1)
            .ToList();
        
        var frequencies = new List<Frequency>();
        
        foreach (var actualDeparture in departures)
        {
            var previousDeparture = GetPreviousDeparture(actualDeparture, departures);
            var nextDeparture = GetNextDeparture(actualDeparture, departures);
            var frequencyAverage = CalculateFrequencyAverage(previousDeparture, actualDeparture, nextDeparture);
            
            // Para verificar que las frequencias tengan alguna coeerencia 
            if (frequencyAverage > new TimeSpan(24, 0, 0))
                if (frequencies.Where(freq => freq.VariantId == actualDeparture.VariantId).Count() > 1)
                    frequencyAverage = new TimeSpan(0, 5, 0);

            var actualFrequency = new Frequency
            {
                VariantId = actualDeparture.VariantId,
                DayType = actualDeparture.DayType,
                DepartureTime = actualDeparture.Hour,
                FrequencyAverage = frequencyAverage
            };
            
            frequencies.Add(actualFrequency);
        }
        
        return frequencies;
    }

    private static PlannedRouteCsvLine GetPreviousDeparture(PlannedRouteCsvLine actualDeparture, List<PlannedRouteCsvLine> departures)
    {
        var previousStop = departures
            .Where(departure => departure.VariantId == actualDeparture.VariantId &&
                                departure.DayType == actualDeparture.DayType &&
                                departure.Frequency < actualDeparture.Frequency)
            .OrderByDescending(departure => departure.Frequency)
            .FirstOrDefault();
        
        if (previousStop != null) return previousStop;
        
        previousStop = departures
            .Where(departure => departure.VariantId == actualDeparture.VariantId && 
                                departure.DayType == actualDeparture.DayType.GetPreviousDayType())
            .OrderByDescending(departure => departure.Frequency)
            .FirstOrDefault();
        
        if (previousStop != null) return previousStop;
        
        previousStop = departures
            .Where(departure => departure.VariantId == actualDeparture.VariantId && 
                                departure.DayType == actualDeparture.DayType.GetPreviousDayType().GetPreviousDayType())
            .OrderByDescending(departure => departure.Frequency)
            .FirstOrDefault();
        
        return previousStop ?? actualDeparture;
    }
    
    private static PlannedRouteCsvLine GetNextDeparture(PlannedRouteCsvLine actualDeparture, List<PlannedRouteCsvLine> departures)
    {
        var nextStop = departures
            .Where(departure => departure.VariantId == actualDeparture.VariantId &&
                                departure.DayType == actualDeparture.DayType &&
                                departure.Frequency > actualDeparture.Frequency)
            .OrderBy(departure => departure.Frequency)
            .FirstOrDefault();
        
        if (nextStop != null) return nextStop;
        
        nextStop = departures
            .Where(departure => departure.VariantId == actualDeparture.VariantId && 
                                departure.DayType == actualDeparture.DayType.GetNextDayType())
            .OrderBy(departure => departure.Frequency)
            .FirstOrDefault();
        
        if (nextStop != null) return nextStop;
        
        nextStop = departures
            .Where(departure => departure.VariantId == actualDeparture.VariantId && 
                                departure.DayType == actualDeparture.DayType.GetNextDayType().GetNextDayType())
            .OrderBy(departure => departure.Frequency)
            .FirstOrDefault();
        
        return nextStop ?? actualDeparture;
    }
    
    private static TimeSpan CalculateFrequencyAverage(PlannedRouteCsvLine previousDeparture, PlannedRouteCsvLine actualDeparture, PlannedRouteCsvLine nextDeparture)
    {
        var previousDepartureDateTime = new DateTime(2000, 1, 1, previousDeparture.Frequency.Hour, previousDeparture.Frequency.Minute, 0);
        var actualDepartureDateTime = new DateTime(2000, 1, 1, actualDeparture.Frequency.Hour, actualDeparture.Frequency.Minute, 0);
        var nextDepartureDateTime = new DateTime(2000, 1, 1, nextDeparture.Frequency.Hour, nextDeparture.Frequency.Minute, 0);
        
        var previousDepartureDayType = previousDeparture.DayType;
        var actualDepartureDayType = actualDeparture.DayType;
        var nextDepartureDayType = nextDeparture.DayType;

        while (previousDepartureDayType != actualDepartureDayType)
        {
            previousDepartureDateTime = previousDepartureDateTime.AddDays(-1);
            previousDepartureDayType = previousDepartureDayType.GetNextDayType();
        }
        
        while (nextDepartureDayType != actualDepartureDayType)
        {
            nextDepartureDateTime = nextDepartureDateTime.AddDays(1);
            nextDepartureDayType = nextDepartureDayType.GetPreviousDayType();
        }
        
        return TimeSpan.FromTicks(Math.Min(
            (actualDepartureDateTime - previousDepartureDateTime).Ticks > 0
                ? (actualDepartureDateTime - previousDepartureDateTime).Ticks
                : long.MaxValue,
            (nextDepartureDateTime - actualDepartureDateTime).Ticks > 0
                ? (nextDepartureDateTime - actualDepartureDateTime).Ticks
                : long.MaxValue)) / 2;;
    }
    
    public static TimeSpan GetFrequencyAverage(List<Frequency> frequencies, Ticket ticket, DateTime estimatedDateTimeOfDeparture)
    {
        var dayType = ticket.SoldDate.GetDateType();
        if (ticket.SoldDate.Date > estimatedDateTimeOfDeparture.Date) dayType = dayType.GetPreviousDayType();
        
        var frequency = frequencies
            .Where(f => f.VariantId == ticket.VariantId && f.DayType == dayType)
            .OrderBy(f => Math.Abs((f.DepartureTime.ToTimeSpan() - estimatedDateTimeOfDeparture.TimeOfDay).Ticks))
            .FirstOrDefault();
        
        return frequency.FrequencyAverage;
    }
    
    private static TimeOnly ToTimeOnly(DateTime date)
    {
        var timeOnly = new TimeOnly(date.Hour, date.Minute, date.Second);
        return timeOnly;
    }
}