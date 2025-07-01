using HPCConcept.Structures;

namespace HPCConcept.Auxiliary;

public class PlannedRouteCsvLine
{
    public DayType DayType { get; set; }
    public int VariantId { get; set; }
    public TimeOnly FrequencyValue { get; set; }
    public int StopId { get; set; }
    public int RelativeStopId { get; set; }
    public TimeOnly Hour { get; set; }
    public char LastDay { get; set; }
    public TimeSpan? TimeFromLastStop { get; set; } = null;
}