using HPCConcept.Auxiliary;
using HPCConcept.Structures;

namespace HPCConcept
{
    public static class CsvReader
    {
        public static List<PlannedRouteCsvLine> ReadPlannedRouteCsv(string filePath)
        {
            var plannedRoutes = new List<PlannedRouteCsvLine>();

            using var reader = new StreamReader(filePath);
            reader.ReadLine();

            while (reader.ReadLine() is { } line)
            {
                var parts = line.Split(';');
                if (parts.Length < 6) continue;
                
                var relativeStopId = int.Parse(parts[4]);
                TimeOnly? departureTime = relativeStopId == 1 ? TimeOnly.Parse(int.Parse(parts[4]) == 1 ? 
                    parts[5].PadLeft(4, '0').Insert(2, ":") :
                    parts[2].PadLeft(5, '0').Substring(0, 4).Insert(2, ":")) : null;

                var plannedRoute = new PlannedRouteCsvLine
                {
                    DayType = Enum.Parse<DayType>(parts[0]),
                    VariantId = int.Parse(parts[1]),
                    Frequency = TimeOnly.Parse(parts[2].PadLeft(5, '0').Substring(0, 4).Insert(2, ":")),
                    DepartureTime = departureTime, 
                    StopId = int.Parse(parts[3]),
                    RelativeStopId = relativeStopId,
                    Hour = TimeOnly.Parse(parts[5].PadLeft(4, '0').Insert(2, ":")),
                    LastDay = parts.Length > 6 ? parts[6][0] : 'N'
                };

                plannedRoutes.Add(plannedRoute);
            }

            return plannedRoutes;
        }
        
        public static List<Ticket> ReadTicketsCsv(string filePath)
        {
            var tickets = new List<Ticket>();

            using var reader = new StreamReader(filePath);
            reader.ReadLine();

            while (reader.ReadLine() is { } line)
            {
                var parts = line.Split(',');
                if (parts.Length < 3) continue;

                var ticket = new Ticket
                {
                    SoldDate = DateTime.Parse(parts[0]),
                    StopId = int.Parse(parts[1]),
                    VariantId = int.Parse(parts[2])
                };

                tickets.Add(ticket);
            }

            return tickets.OrderBy(ticket => ticket.SoldDate).ToList();
        }
    }
}