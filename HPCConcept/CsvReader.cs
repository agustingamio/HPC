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

                var plannedRoute = new PlannedRouteCsvLine
                {
                    DayType = Enum.Parse<DayType>(parts[0]),
                    VariantId = int.Parse(parts[1]),
                    FrequencyValue = TimeOnly.Parse(parts[2].PadLeft(5, '0').Substring(0, 4).Insert(2, ":")),
                    StopId = int.Parse(parts[3]),
                    RelativeStopId = int.Parse(parts[4]),
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
                if (parts.Length < 17) continue;

                var ticket = new Ticket
                {
                    SoldDate = DateTime.Parse(parts[2]),
                    StopId = int.Parse(parts[11]),
                    VariantId = int.Parse(parts[16])
                };

                tickets.Add(ticket);
            }

            return tickets.OrderBy(ticket => ticket.SoldDate).ToList();
        }
    }
}