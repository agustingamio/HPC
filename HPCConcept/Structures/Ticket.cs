using HPCConcept.Helper;
using System.ComponentModel.Design;

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
        
            stop.AddLastSoldTicket(ticket.SoldDate);

            if (stop.RelativeStopId == 1) return;
        
            var previousStops = StopGraph.GetPreviousStops(stop, graph);
            var estimatedDateTimeOfDeparture = ticket.SoldDate - GetEstimatedTimeBetweenStops(previousStops);
            
            var range = Frequency.GetFrequencyAverage(frequencies, ticket, estimatedDateTimeOfDeparture);

            var stopsToUpdate = GetStopsToUpdate(previousStops, range, ticket);
            
            if (stopsToUpdate.Item2 == null) return;
            
            var timeFromLastSoldTicket = ticket.SoldDate - stopsToUpdate.Item2;
            var theoreticalTimeFromLastStop = stopsToUpdate.Item1.Skip(1).Sum(stopToSum => stopToSum.TimeFromLastStop.Value.TotalSeconds);

            foreach (var stopToUpdate in stopsToUpdate.Item1.Skip(1))
            {
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
    
    private static (List<StopGraph>, DateTime?) GetStopsToUpdate(List<StopGraph> stops, TimeSpan frequency, Ticket actualTicket)
    {
        var stopsToUpdate = new List<StopGraph> {stops.First()};
        var estimatedCommuteTime = stops.First().TimeFromLastStop;
        
        foreach (var stop in stops.Skip(1))
        {
            stopsToUpdate.Insert(0, stop);
            if (stop.RelativeStopId == 1) continue;

            foreach (var ticket in stop.LastSoldTickets)
            {
                var actualTicketDateWithCommuteTime = actualTicket.SoldDate - estimatedCommuteTime;
                var ticketDateWithInferiorRange = ticket - frequency;
                var ticketDateWithSuperiorRange = ticket + frequency;
                
                if (actualTicketDateWithCommuteTime > ticketDateWithInferiorRange &&
                    actualTicketDateWithCommuteTime < ticketDateWithSuperiorRange)
                {
                    return (stopsToUpdate, ticket.Date);
                }
            }
            
            estimatedCommuteTime += stop.TimeFromLastStop;
        }

        return (stopsToUpdate, null);
    }

    private static TimeSpan GetEstimatedTimeBetweenStops(List<StopGraph> graph)
    {
        return graph.Aggregate(TimeSpan.Zero, (current, stop) => current + stop.TimeFromLastStop.Value);
    }
}