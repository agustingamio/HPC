﻿using HPCConcept;
using HPCConcept.Structures;
using Serilog;
using System.IO;
using System.Text;

Log.Logger = new LoggerConfiguration()
    .WriteTo.File(@"C:\FING\HPC\ConsoleApp\HPCConcept\log.txt")
    .CreateLogger();

Log.Information("Starting application...");
var frequencies = Frequency.CalculateFrequencyFromCsv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv");

Log.Information("Frequencies loaded. Creating the stop graph...");
var graph = StopGraph.CreateStopGraphFromCsv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\uptu_pasada_variante.csv");

Log.Information("Stop graph created. Reading tickets from CSV...");
var tickets = CsvReader.ReadTicketsCsv("C:\\FING\\HPC\\ConsoleApp\\HPCConcept\\tickets_output.csv");

// Save tickets to CSV
// var outputPath = @"C:\FING\HPC\ConsoleApp\HPCConcept\tickets_output.csv";
// using (var writer = new StreamWriter(outputPath, false, Encoding.UTF8))
// {
//     // Optionally write header
//     writer.WriteLine("Column1,Column2,Column3"); // Replace with actual column names
//     foreach (var ticket in tickets)
//     {
//         writer.WriteLine(ticket.ToCsvLine()); // Implement ToCsvLine() in your Ticket class
//     }
// }

Log.Information("Starting to process tickets...");
var count = 0;
var returnTypes = new int[7] {0, 0, 0, 0, 0, 0, 0};
foreach (var ticket in tickets)
{
    count++;
    Console.Write("Starting proccess of: ");
    ticket.PrintTicketLine();
    var result = Ticket.ProcessTicket(ticket, graph, frequencies, Log.Logger);
    //printf("Ending proccessing... \n");
    Console.WriteLine($"Ending proccessing...");
    Console.WriteLine();
    returnTypes[result]++;
    
    if (count % 1000 == 0)
    {
        Log.Information($"Processed {count} tickets so far.");
        Log.Information("Ticket processing results: " +
                        $"Success: {returnTypes[(int)ReturnType.Success]}, " +
                        $"VariantNotFound: {returnTypes[(int)ReturnType.VariantNotFound]}, " +
                        $"StopForVariantNotFound: {returnTypes[(int)ReturnType.StopForVariantNotFound]}, " +
                        $"NoStopForVariantAndDate: {returnTypes[(int)ReturnType.NoStopForVariantAndDate]}, " +
                        $"Error: {returnTypes[(int)ReturnType.Error]}, " +
                        $"NoPreviousStops: {returnTypes[(int)ReturnType.NoPreviousStops]}, " +
                        $"NoPreviousStopsWithSoldTicket: {returnTypes[(int)ReturnType.NoPreviousStopsWithSoldTicket]}");
    }
}

public enum ReturnType
{
    Success,
    VariantNotFound,
    StopForVariantNotFound,
    NoStopForVariantAndDate,
    Error,
    NoPreviousStops,
    NoPreviousStopsWithSoldTicket,
}