using HPCConcept.Helper;

namespace HPCConcept.Structures;

public class Frequency
{
    public int VariantId { get; set; }
    public DayType DayType { get; set; }
    public TimeRange TimeRange { get; set; }
    public TimeSpan FrequencyAverage { get; set; }
    
    public static List<Frequency> CalculateFrequencyFromCsv(string filePath)
    {
        var plannedRoutes = CsvReader.ReadPlannedRouteCsv(filePath);
        
        var groupedPlannedRoutes = plannedRoutes
            .Where(route => route.RelativeStopId == 1)
            .GroupBy(route => new
            {
                route.DayType, 
                route.VariantId, 
                TimeRange = TimeRangeHelper.GetTimeRange(route.Hour)
            })
            .ToList();
        
        var frequencies = new List<Frequency>();
        
        foreach (var group in groupedPlannedRoutes)
        {
            var dayType = group.Key.DayType;
            var variantId = group.Key.VariantId;
            var timeRange = group.Key.TimeRange;

            var timeRanges = group
                .Select(route => route.FrequencyValue)
                .Distinct()
                .OrderBy(time => time)
                .ToList();

            for (var i = 0; i < timeRanges.Count - 1; i++)
            {
                var startTime = timeRanges[i];
                var endTime = timeRanges[i + 1];

                var frequencyAverage = (endTime - startTime);

                frequencies.Add(new Frequency
                {
                    VariantId = variantId,
                    DayType = dayType,
                    TimeRange = timeRange,
                    FrequencyAverage = frequencyAverage
                });
            }
        }
        
        return frequencies;
    } 
}