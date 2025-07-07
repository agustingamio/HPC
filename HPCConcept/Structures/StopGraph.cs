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
    public List<DateTime> LastSoldTickets { get; } = [];
    
    public void AddLastSoldTicket(DateTime ticket)
    {
        if (LastSoldTickets.Count == 10) LastSoldTickets.RemoveAt(0);
        LastSoldTickets.Insert(0, ticket);
    }
    
    public static List<StopGraph> CreateStopGraphFromCsv(string filePath)
    {
        var stopGraphs = new List<StopGraph>();
        var routes = GetPlannerRoutesFromCsv(filePath);
        
        var routeLookup = routes
            .GroupBy(route => new { route.VariantId, route.DayType, route.StopId })
            .ToDictionary(group => group.Key);
        
        foreach (var filteredRoute in routeLookup.Values)
        {
            var averageTimeFromLastStop = filteredRoute.Average(route => route.TimeFromLastStop.Value.TotalMinutes);
            var stopGraph = new StopGraph
            {
                StopId = filteredRoute.Key.StopId,
                VariantId = filteredRoute.Key.VariantId,
                RelativeStopId = filteredRoute.First().RelativeStopId,
                TimeFromLastStop = TimeSpan.FromMinutes(averageTimeFromLastStop),
                DayType = filteredRoute.Key.DayType
            };
            stopGraphs.Add(stopGraph);
        }

        return stopGraphs;
    }
    
    private static List<PlannedRouteCsvLine> GetPlannerRoutesFromCsv(string filePath)
    {
        var plannedRoutes = CsvReader.ReadPlannedRouteCsv(filePath);

        var routeLookup = plannedRoutes
            .GroupBy(route => new { route.VariantId, FrequencyValue = route.Frequency, route.DayType, route.LastDay })
            .ToDictionary(
                group => group.Key,
                group => group.OrderBy(route => route.RelativeStopId).ToList()
            );
        
        var routeLookupWithOutLastDay = plannedRoutes
            .GroupBy(route => new { route.VariantId, FrequencyValue = route.Frequency, route.DayType })
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
                        var key1 = new { actualStop.VariantId, FrequencyValue = actualStop.Frequency, DayType = actualStop.DayType.GetPreviousDayType(), LastDay = 'N' };
                        routeLookup.TryGetValue(key1, out previousStops);
                        previousStop = previousStops[actualStop.RelativeStopId - 2];
                        dayTypeChanged = true;

                        break;
                    }
                    case 'S':
                    {
                        var key2 = new { actualStop.VariantId, FrequencyValue = actualStop.Frequency, actualStop.DayType, LastDay = 'S' };
                        routeLookup.TryGetValue(key2, out previousStops);
                        previousStop = previousStops[actualStop.RelativeStopId - 1 - previousStops[0].RelativeStopId];
                        dayTypeChanged = true;

                        break;
                    }
                    case '*':
                        var key3 = new { actualStop.VariantId, FrequencyValue = actualStop.Frequency, actualStop.DayType, LastDay = '*' };
                        routeLookup.TryGetValue(key3, out previousStops);

                        if (previousStops[0].RelativeStopId == actualStop.RelativeStopId)
                        {
                            var key4 = new { actualStop.VariantId, FrequencyValue = actualStop.Frequency, actualStop.DayType };
                            routeLookupWithOutLastDay.TryGetValue(key4, out previousStops);
                        }
                        
                        previousStop = previousStops[actualStop.RelativeStopId - 1 - previousStops[0].RelativeStopId];
                        
                        break;
                    default:
                    {
                        var key4 = new { actualStop.VariantId, FrequencyValue = actualStop.Frequency, actualStop.DayType, LastDay = 'N' };
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

    public static StopGraph GetPreviousStop(StopGraph actualStop, List<StopGraph> graph)
    {
        List<StopGraph> previousStops;
        
        previousStops = graph.Where(stop => stop.VariantId == actualStop.VariantId &&
                                                        stop.RelativeStopId == actualStop.RelativeStopId - 1 &&
                                                        stop.DayType == actualStop.DayType).ToList();
        
        if (previousStops.Count > 1) return previousStops[0];
        if (previousStops.Count == 1) return previousStops[0];
        
        previousStops = graph.Where(stop => stop.VariantId == actualStop.VariantId && 
                                                        stop.RelativeStopId == actualStop.RelativeStopId - 1 && 
                                                        stop.DayType == actualStop.DayType.GetPreviousDayType()).ToList();

        if (previousStops.Count > 1) return previousStops[0];
        if (previousStops.Count == 1) return previousStops[0];
        
        throw new Exception($"No previous stop found for stop {actualStop.StopId} with variant {actualStop.VariantId} and relative stop id {actualStop.RelativeStopId}.");
    }

    public static List<StopGraph> GetPreviousStops(StopGraph actualStop, List<StopGraph> graph)
    {
        var result = new List<StopGraph> { actualStop };

        for (var i = actualStop.RelativeStopId - 1; i > 0; i--)
        {
            var previousStop = GetPreviousStop(actualStop, graph);
            actualStop = previousStop;
            result.Add(actualStop);
        }
        
        return result;
    }
}

