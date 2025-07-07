using HPCConcept;
using HPCConcept.Structures;
using Serilog;

Log.Logger = new LoggerConfiguration()
    .WriteTo.File(@"C:\FING\HPC\ConsoleApp\HPCConcept\log.txt")
    .CreateLogger();

Log.Information("Starting application...");
var frequencies = Frequency.CalculateFrequencyFromCsv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv");

Log.Information("Frequencies loaded. Creating the stop graph...");
var graph = StopGraph.CreateStopGraphFromCsv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv");

Log.Information("Stop graph created. Reading tickets from CSV...");
var tickets = CsvReader.ReadTicketsCsv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\viajes_stm_042025.csv");

Log.Information("Starting to process tickets...");
var count = 0;
var returnTypes = new int[4] {0, 0, 0, 0};
foreach (var ticket in tickets)
{
    count++;
    var result = Ticket.ProcessTicket(ticket, graph, frequencies, Log.Logger);
    returnTypes[result]++;
    
    if (count % 1000 == 0)
    {
        Log.Information($"Processed {count} tickets so far.");
        Log.Information($"Return types: Success: {returnTypes[(int)ReturnType.Success]}, Error: {returnTypes[(int)ReturnType.Error]}, NoPreviousStops: {returnTypes[(int)ReturnType.NoPreviousStops]}, NoPreviousStopsWithSoldTicket: {returnTypes[(int)ReturnType.NoPreviousStopsWithSoldTicket]}");
    }
}

public enum ReturnType
{
    Success,
    Error,
    NoPreviousStops,
    NoPreviousStopsWithSoldTicket,
}