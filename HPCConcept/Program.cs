using HPCConcept;
using HPCConcept.Structures;

var frequencies = Frequency.CalculateFrequencyFromCsv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv");
var graph = StopGraph.CreateStopGraphFromCsv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv");
var tickets = CsvReader.ReadTicketsCsv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\viajes_stm_042025.csv");

foreach (var ticket in tickets)
{
    Ticket.ProcessTicket(ticket, graph, frequencies);
}