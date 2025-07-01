using HPCConcept.Auxiliary;
using HPCConcept.Helper;

namespace HPCConcept.Structures;

public class StopGraph
{
    public int StopId { get; set; }
    public int VariantId { get; set; }
    public int RelativeStopId { get; set; }
    public DayType DayType { get; set; }
    public TimeSpan? TimeFromLastStop { get; set; }
    public DateTime? LastSoldTicket { get; set; }
    public Queue<DateTime> LastSoldTickets { get; set; } = new Queue<DateTime>();
    

    public static List<StopGraph> CreateStopGraphFromCsv(string filePath)
    {
        var routes = GetPlannerRoutesFromCsv(filePath);
        
        var routeLookup = routes
            .GroupBy(route => new { route.VariantId, route.DayType, route.StopId })
            .ToDictionary(group => group.Key);

        return (from filteredRoute in routeLookup.Values
            let averageTimeFromLastStop = filteredRoute.Average(route => route.TimeFromLastStop.Value.TotalMinutes)
            select new StopGraph
            {
                StopId = filteredRoute.Key.StopId,
                VariantId = filteredRoute.Key.VariantId,
                RelativeStopId = filteredRoute.First().RelativeStopId,
                TimeFromLastStop = TimeSpan.FromMinutes(averageTimeFromLastStop),
                LastSoldTicket = null,
                DayType = filteredRoute.Key.DayType
            }).ToList();
    }
    
    private static List<PlannedRouteCsvLine> GetPlannerRoutesFromCsv(string filePath)
    {
        var plannedRoutes = CsvReader.ReadPlannedRouteCsv(filePath);

        var routeLookup = plannedRoutes
            .GroupBy(route => new { route.VariantId, route.FrequencyValue, route.DayType, route.LastDay })
            .ToDictionary(
                group => group.Key,
                group => group.OrderBy(route => route.RelativeStopId).ToList()
            );
        
        var routeLookupWithOutLastDay = plannedRoutes
            .GroupBy(route => new { route.VariantId, route.FrequencyValue, route.DayType })
            .ToDictionary(
                group => group.Key,
                group => group.OrderBy(route => route.RelativeStopId).ToList()
            );

        foreach (var filteredRoute in routeLookup)
        {
            var dayTypeChanged = false;

            foreach (var actualStop in filteredRoute.Value)
            {                
                if (actualStop.RelativeStopId == 1)
                {
                    actualStop.TimeFromLastStop = new TimeSpan(0, 0, 0);
                    continue;
                }

                List<PlannedRouteCsvLine>? previousStops;
                PlannedRouteCsvLine? previousStop;

                switch (actualStop.LastDay)
                {
                    case 'S' when !dayTypeChanged:
                    {
                        var key1 = new { actualStop.VariantId, actualStop.FrequencyValue, DayType = actualStop.DayType.GetPreviousDayType(), LastDay = 'N' };
                        routeLookup.TryGetValue(key1, out previousStops);
                        previousStop = previousStops[actualStop.RelativeStopId - 2];
                        dayTypeChanged = true;

                        break;
                    }
                    case 'S':
                    {
                        var key2 = new { actualStop.VariantId, actualStop.FrequencyValue, actualStop.DayType, LastDay = 'S' };
                        routeLookup.TryGetValue(key2, out previousStops);
                        previousStop = previousStops[actualStop.RelativeStopId - 1 - previousStops[0].RelativeStopId];
                        dayTypeChanged = true;

                        break;
                    }
                    case '*':
                        var key3 = new { actualStop.VariantId, actualStop.FrequencyValue, actualStop.DayType, LastDay = '*' };
                        routeLookup.TryGetValue(key3, out previousStops);

                        if (previousStops[0].RelativeStopId == actualStop.RelativeStopId)
                        {
                            var key4 = new { actualStop.VariantId, actualStop.FrequencyValue, actualStop.DayType };
                            routeLookupWithOutLastDay.TryGetValue(key4, out previousStops);
                        }
                        
                        previousStop = previousStops[actualStop.RelativeStopId - 1 - previousStops[0].RelativeStopId];
                        
                        break;
                    default:
                    {
                        var key4 = new { actualStop.VariantId, actualStop.FrequencyValue, actualStop.DayType, LastDay = 'N' };
                        routeLookup.TryGetValue(key4, out previousStops);
                        previousStop = previousStops[actualStop.RelativeStopId - 2];
                        break;
                    }
                }
                
                actualStop.TimeFromLastStop = actualStop.Hour - previousStop.Hour;
            }
        }
        
        return plannedRoutes;
    }
}

