using HPCConcept.Helper;

namespace HPCConcept.Structures;

public class Ticket
{
    public DateTime SoldDate { get; set; }
    public int StopId { get; set; }
    public int VariantId { get; set; }
    
    public static void ProcessTicket(Ticket ticket, List<StopGraph> graph, List<Frequency> frequencies)
    {
        try
        {
            var stop = graph.FirstOrDefault(stop => stop.StopId == ticket.StopId && 
                                                    stop.VariantId == ticket.VariantId && 
                                                    stop.DayType == ticket.SoldDate.GetDateType());
        
            stop.LastSoldTicket = ticket.SoldDate;

            if (stop.RelativeStopId == 1) return;
        
            // Get the previous stop where a ticket was sold
            var stopsToUpdate = GetPreviousStopWithSoldTicket(stop, graph, frequencies);
        
            // Update previous stops
            // TODO: Capas en la funcion anterior ya prodria devovler una lista con todas las paradas que se van a tener que cambiar, asi seria mas facil.
            
            var timeFromLastSoldTicket = ticket.SoldDate - stopsToUpdate.Last().LastSoldTicket;
            var theoreticalTimeFromLastStop = stopsToUpdate.Sum(stopToSum => stopToSum.TimeFromLastStop.Value.TotalSeconds);

            foreach (var stopToUpdate in stopsToUpdate)
            {
                if (stopToUpdate.RelativeStopId == 1) continue;
            
                var percentage = stopToUpdate.TimeFromLastStop.Value.TotalSeconds * 100 / theoreticalTimeFromLastStop;
                var timeToAdd = timeFromLastSoldTicket.Value.TotalSeconds * percentage / 100;
                stopToUpdate.TimeFromLastStop = TimeSpan.FromSeconds(( stopToUpdate.TimeFromLastStop.Value.TotalSeconds + timeToAdd ) / 2);
            }
        }
        catch (Exception e)
        {
            Console.WriteLine(e);
        }
        
    }

    private static List<StopGraph> GetPreviousStopWithSoldTicket(StopGraph actualStop, List<StopGraph> graph, List<Frequency> frequencies)
    {
        var result = new List<StopGraph> { actualStop };
        var relativeStopId = actualStop.RelativeStopId;
        
        var range = frequencies.FirstOrDefault(frequency => frequency.VariantId == actualStop.VariantId && 
                                                            frequency.DayType == actualStop.DayType && 
                                                            frequency.TimeRange == ToTimeOnly((DateTime)actualStop.LastSoldTicket).GetTimeRange()).FrequencyAverage / 2;
        
        while (true)
        {
            var previousStop = graph.FirstOrDefault(stop => stop.VariantId == actualStop.VariantId &&
                                                            stop.RelativeStopId == relativeStopId - 1 &&
                                                            stop.DayType == actualStop.DayType) ?? 
                               graph.FirstOrDefault(stop => stop.VariantId == actualStop.VariantId && 
                                                            stop.RelativeStopId == relativeStopId - 1 && 
                                                            stop.DayType == actualStop.DayType.GetPreviousDayType());
            
            result.Add(previousStop);
            
            if (previousStop.RelativeStopId == 1) return result;
            
            if (previousStop.LastSoldTicket != null && 
                actualStop.LastSoldTicket < previousStop.LastSoldTicket + range &&
                actualStop.LastSoldTicket > previousStop.LastSoldTicket - range)
                return result;

            relativeStopId--;
        }
    }

    private static TimeOnly ToTimeOnly(DateTime date)
    {
        return new TimeOnly(date.Hour, date.Minute, date.Second);
    }
}