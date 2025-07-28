using HPCConcept.Helper;
using Serilog;
using System.Text;

namespace HPCConcept.Structures;

public class Ticket
{
    public DateTime SoldDate { get; set; }
    public int StopId { get; set; }
    public int VariantId { get; set; }
    
    public static int ProcessTicket(Ticket ticket, List<StopGraph> graph, List<Frequency> frequencies, ILogger logger, StringBuilder outputBuilder)
    {
        try
        {
            var stop = graph.FirstOrDefault(stop => stop.StopId == ticket.StopId && 
                                                    stop.VariantId == ticket.VariantId && 
                                                    stop.DayType == ticket.SoldDate.GetDateType());
            

            if (stop == null) throw new Exception();
            outputBuilder.AppendLine(stop.TextStopGraphLine());

            if (!stop.LastSoldTickets.Any(t => Math.Abs((t - ticket.SoldDate).TotalMinutes) < 1))
            {
                stop.AddLastSoldTicket(ticket.SoldDate);
                outputBuilder.Append("Ticket added to stop: ");
            }
            else
            {
                outputBuilder.Append("Ticket NOT added to stop: ");
            }

            if (stop.RelativeStopId == 1) return 5;
        
            var previousStops = StopGraph.GetPreviousStops(stop, graph);
            outputBuilder.AppendLine($"Previous stops count: {previousStops.Count}");
            var estimatedDateTimeOfDeparture = ticket.SoldDate - GetEstimatedTimeBetweenStops(previousStops);
            outputBuilder.AppendLine($"Estimated departure time: {estimatedDateTimeOfDeparture:HH:mm:ss}");
            
            var range = Frequency.GetFrequencyAverage(frequencies, ticket, estimatedDateTimeOfDeparture);
            outputBuilder.AppendLine($"Frequency range: {range.TotalSeconds} seconds");

            var stopsToUpdate = GetStopsToUpdate(previousStops, range, ticket);
            outputBuilder.AppendLine($"Stops to update count: {stopsToUpdate.Item1.Count}");
            if (stopsToUpdate.Item2.HasValue)
            {
                outputBuilder.AppendLine($"Ticket found for update");
            }
            else
            {
                outputBuilder.AppendLine($"No matching ticket found");
            }
            
            if (stopsToUpdate.Item2 == null) return 6;
            
            var timeFromLastSoldTicket = ticket.SoldDate - stopsToUpdate.Item2;
            outputBuilder.AppendLine($"Time from last sold ticket: {timeFromLastSoldTicket.Value.TotalSeconds} seconds");
            var theoreticalTimeFromLastStop = stopsToUpdate.Item1.Skip(1).Sum(stopToSum => stopToSum.TimeFromLastStop.Value.TotalSeconds);
            outputBuilder.AppendLine($"Theoretical time: {theoreticalTimeFromLastStop} seconds");
 
            foreach (var stopToUpdate in stopsToUpdate.Item1.Skip(1))
            {
                double percentage = 100;;
                if (theoreticalTimeFromLastStop != 0)
                {
                    percentage = stopToUpdate.TimeFromLastStop.Value.TotalSeconds * 100 / theoreticalTimeFromLastStop;
                }
                
                var timeToAdd = timeFromLastSoldTicket.Value.TotalSeconds * percentage / 100;
                if (double.IsNaN(timeToAdd))  timeToAdd = 0;
                stopToUpdate.TimeFromLastStop = TimeSpan.FromSeconds(( stopToUpdate.TimeFromLastStop.Value.TotalSeconds + timeToAdd ) / 2);
                outputBuilder.AppendLine($"Updated time for stop {stopToUpdate.StopId}: {stopToUpdate.TimeFromLastStop.Value.TotalSeconds} seconds");
            }

            return 0;
        }
        catch (Exception e)
        {
            if (!graph.Exists(stop => stop.VariantId == ticket.VariantId))
            {
                logger.Error(e, "No se encuentra la variable Nro: {VariantId}", ticket.VariantId);
                return 1;
            }

            if (!graph.Exists(stop => stop.VariantId == ticket.VariantId &&
                                     stop.StopId == ticket.StopId))
            {
                logger.Error(e, "No se encuentra la parada Nro: {StopId} para la variante Nro: {VariantId}", ticket.StopId, ticket.VariantId);
                return 2;
            }
            
            if (!graph.Exists(stop => stop.VariantId == ticket.VariantId &&
                                        stop.StopId == ticket.StopId &&
                                        stop.DayType == ticket.SoldDate.GetDateType()))
            {
                logger.Error(e, "No se encuentra la parada Nro: {StopId} para la variante Nro: {VariantId} en el dia {DayType}", ticket.StopId, ticket.VariantId, ticket.SoldDate.GetDateType());
                return 3;
            }
            
            logger.Error(e, e.Message);
            return 4;
        }
    }
    
    private static (List<StopGraph>, DateTime?) GetStopsToUpdate(List<StopGraph> stops, TimeSpan frequency, Ticket actualTicket)
    {
        var stopsToUpdate = new List<StopGraph> {stops.First()};
        var estimatedCommuteTime = stops.First().TimeFromLastStop;
        
        foreach (var stop in stops.Skip(1))
        {
            stopsToUpdate.Insert(0, stop);

            foreach (var ticket in stop.LastSoldTickets)
            {
                var actualTicketDateWithCommuteTime = actualTicket.SoldDate - estimatedCommuteTime;

                DateTime ticketDateWithInferiorRange;
                DateTime ticketDateWithSuperiorRange;
                if (frequency > new TimeSpan(10, 0, 0, 0))
                {
                    //TODO: Verificar si entra aca, no deberia
                    ticketDateWithInferiorRange = ticket.Date.AddDays(-1);
                    ticketDateWithSuperiorRange = ticket.Date.AddDays(1);
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
    
    // Implementation of ToCsvLine method to convert Ticket to CSV format
    public string ToCsvLine()
    {
        return $"{SoldDate:yyyy-MM-dd HH:mm:ss},{StopId},{VariantId}";
    }
    
    public string TextTicketLine()
    {
        return $"Sold Date: {SoldDate:yyyy-MM-dd HH:mm:ss}, Stop ID: {StopId}, Variant ID: {VariantId}";
    }
    
    public void PrintTicketLine()
    {
        Console.WriteLine(
            $"Sold Date: {SoldDate:yyyy-MM-dd HH:mm:ss}, Stop ID: {StopId}, Variant ID: {VariantId}"
        );
    }
}