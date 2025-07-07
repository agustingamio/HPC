using HPCConcept.Helper;
using Serilog;

namespace HPCConcept.Structures;

public class Ticket
{
    public DateTime SoldDate { get; set; }
    public int StopId { get; set; }
    public int VariantId { get; set; }
    
    public static int ProcessTicket(Ticket ticket, List<StopGraph> graph, List<Frequency> frequencies, ILogger logger)
    {
        try
        {
            var stop = graph.FirstOrDefault(stop => stop.StopId == ticket.StopId && 
                                                    stop.VariantId == ticket.VariantId && 
                                                    stop.DayType == ticket.SoldDate.GetDateType());
            
            if (stop == null) throw new Exception($"No se encontro la parada Nro: {ticket.StopId} para la variante Nro: {ticket.VariantId} en el dia {ticket.SoldDate.GetDateType()}");
            
            if (!stop.LastSoldTickets.Any(t => Math.Abs((t - ticket.SoldDate).TotalMinutes) < 1))
                stop.AddLastSoldTicket(ticket.SoldDate);

            if (stop.RelativeStopId == 1) return 2;
        
            var previousStops = StopGraph.GetPreviousStops(stop, graph);
            var estimatedDateTimeOfDeparture = ticket.SoldDate - GetEstimatedTimeBetweenStops(previousStops);
            
            var range = Frequency.GetFrequencyAverage(frequencies, ticket, estimatedDateTimeOfDeparture);

            var stopsToUpdate = GetStopsToUpdate(previousStops, range, ticket);
            
            if (stopsToUpdate.Item2 == null) return 3;
            
            var timeFromLastSoldTicket = ticket.SoldDate - stopsToUpdate.Item2;
            var theoreticalTimeFromLastStop = stopsToUpdate.Item1.Skip(1).Sum(stopToSum => stopToSum.TimeFromLastStop.Value.TotalSeconds);
 
            foreach (var stopToUpdate in stopsToUpdate.Item1.Skip(1))
            {
                double percentage;
                if (theoreticalTimeFromLastStop != 0)
                {
                    percentage = stopToUpdate.TimeFromLastStop.Value.TotalSeconds * 100 / theoreticalTimeFromLastStop;
                }
                else
                {
                    percentage = 100;
                }
                
                var timeToAdd = timeFromLastSoldTicket.Value.TotalSeconds * percentage / 100;
                if (double.IsNaN(timeToAdd))  timeToAdd = 0;
                stopToUpdate.TimeFromLastStop = TimeSpan.FromSeconds(( stopToUpdate.TimeFromLastStop.Value.TotalSeconds + timeToAdd ) / 2);
            }

            return 0;
        }
        catch (Exception e)
        { 
            logger.Error(e, e.Message);
            return 1;
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

                DateTime ticketDateWithInferiorRange;
                DateTime ticketDateWithSuperiorRange;
                if (frequency > new TimeSpan(10, 0, 0, 0))
                {
                    ticketDateWithInferiorRange = new DateTime(2000, 1, 1, 0, 0, 0);
                    ticketDateWithSuperiorRange = new DateTime(2000, 1, 1, 0, 0, 0);
                }
                else
                {
                    ticketDateWithInferiorRange = ticket - frequency;
                    ticketDateWithSuperiorRange = ticket + frequency;
                }
                
                if (actualTicketDateWithCommuteTime > ticketDateWithInferiorRange &&
                    actualTicketDateWithCommuteTime < ticketDateWithSuperiorRange)
                {
                    return (stopsToUpdate, ticket);
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