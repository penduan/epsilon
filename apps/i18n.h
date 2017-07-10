#ifndef APPS_I18N_H
#define APPS_I18N_H

#include <escher.h>

namespace I18n {
  constexpr static int NumberOfLanguages = 5;
  enum class Message : uint16_t {
    Warning = 0,
    Ok = 1,
    Cancel = 2,
    Next = 3,
    SyntaxError,
    MathError,
    LowBattery,

    /* Variables */
    Variables,
    Number,
    Matrix,
    List,
    Empty,

    /* Toolbox */
    Toolbox,
    AbsoluteValue,
    NthRoot,
    BasedLogarithm,
    Calculation,
    ComplexNumber,
    Probability,
    Arithmetic,
    Matrices,
    Lists,
    Approximation,
    HyperbolicTrigonometry,
    Fluctuation,
    DerivateNumber,
    Integral,
    Sum,
    Product,
    ComplexAbsoluteValue,
    Agument,
    ReelPart,
    ImaginaryPart,
    Conjugate,
    Combination,
    Permutation,
    GreatCommonDivisor,
    LeastCommonMultiple,
    Remainder,
    Quotient,
    Inverse,
    Determinant,
    Transpose,
    Trace,
    Dimension,
    Sort,
    InvSort,
    Maximum,
    Minimum,
    Floor,
    FracPart,
    Ceiling,
    Rounding,
    HyperbolicCosine,
    HyperbolicSine,
    HyperbolicTangent,
    InverseHyperbolicCosine,
    InverseHyperbolicSine,
    InverseHyperbolicTangent,
    Prediction95,
    Prediction,
    Confidence,

    /* Applications */
    Apps,
    AppsCapital,

    /* Calculation */
    CalculApp,
    CalculAppCapital,

    /* Fonction */
    FunctionApp,
    FunctionAppCapital,
    FunctionTab,
    GraphTab,
    ValuesTab,
    Plot,
    DisplayValues,
    FunctionOptions,
    AddFunction,
    DeleteFunction,
    NoFunctionToDelete,
    ActivateDesactivate,
    FunctionColor,
    NoFunction,
    NoActivatedFunction,
    Axis,
    Zoom,
    Initialization,
    Move,
    ToZoom,
    Or,
    Trigonometric,
    RoundAbscissa,
    Orthonormal,
    DefaultSetting,
    PlotOptions,
    Compute,
    Goto,
    ValueNotReachedByFunction,
    Zeros,
    Tangent,
    Intersection,
    SelectLowerBound,
    SelectUpperBound,
    NoZeroFound,
    IntervalSet,
    XStart,
    XEnd,
    Step,
    XColumn,
    FunctionColumn,
    DerivativeColumn,
    DerivativeFunctionColumn,
    ClearColumn,
    CopyColumnInList,
    HideDerivativeColumn,

    /* Sequence */
    SequenceApp,
    SequenceAppCapital,
    SequenceTab,
    AddSequence,
    ChooseSequenceType,
    SequenceType,
    Explicite,
    SingleRecurrence,
    DoubleRecurrence,
    SequenceOptions,
    SequenceColor,
    DeleteSequence,
    NoSequence,
    NoActivatedSequence,
    NStart,
    NEnd,
    TermSum,
    SelectFirstTerm,
    SelectLastTerm,
    ValueNotReachedBySequence,
    NColumn,

  /* Statistics */
    StatsApp,
    StatsAppCapital,
    DataTab,
    HistogramTab,
    BoxTab,
    StatTab,
    Values,
    Sizes,
    ColumnOptions,
    ImportList,
    NoDataToPlot,
    Interval,
    Size,
    Frequency,
    HistogramSet,
    RectangleWidth,
    BarStart,
    FirstQuartile,
    Median,
    ThirdQuartile,
    NoValueToCompute,
    TotalSize,
    Range,
    Mean,
    StandardDeviation,
    Deviation,
    InterquartileRange,
    SquareSum,

    /* Probability  */
    ProbaApp,
    ProbaAppCapital,
    ChooseLaw,
    Binomial,
    Uniforme,
    Exponential,
    Normal,
    Poisson,
    BinomialLaw,
    UniformLaw,
    ExponentialLaw,
    NormalLaw,
    PoissonLaw,
    ChooseParameters,
    RepetitionNumber,
    SuccessProbability,
    IntervalDefinition,
    LambdaExponentialDefinition,
    MeanDefinition,
    DeviationDefinition,
    LambdaPoissonDefinition,
    ComputeProbability,
    ForbiddenValue,
    UndefinedValue,

    /* Regression */
    RegressionApp,
    RegressionAppCapital,
    Regression,
    NoEnoughDataForRegression,
    Reg,
    MeanDot,
    RegressionSlope,
    XPrediction,
    YPrediction,
    ValueNotReachedByRegression,
    NumberOfDots,
    Covariance,

    /* Settings */
    SettingsApp,
    SettingsAppCapital,
    AngleUnit,
    DisplayMode,
    ComplexFormat,
    Language,
    ExamMode,
    ActivateExamMode,
    ExamModeActive,
    ActiveExamModeMessage1,
    ActiveExamModeMessage2,
    ActiveExamModeMessage3,
    ExitExamMode1,
    ExitExamMode2,
    About,
    Degres,
    Radian,
    Auto,
    Scientific,
    Deg,
    Rad,
    Sci,
    SoftwareVersion,
    SerialNumber,
    UpdatePopUp,
    HardwareTestLaunch1,
    HardwareTestLaunch2,
    HardwareTestLaunch3,
    HardwareTestLaunch4,

    /* On boarding */
    UpdateAvailable,
    UpdateMessage1,
    UpdateMessage2,
    UpdateMessage3,
    UpdateMessage4,
    Skip,

    /* UNIVERSAL MESSAGES */
    Default = 0x8000,
    Alpha,
    CapitalAlpha,
    X,
    Y,
    N,
    P,
    Mu,
    Sigma,
    Lambda,
    A,
    B,
    R,
    Sxy,

    XMin,
    XMax,
    YMin,
    YMax,
    YAuto,

    RightIntegralFirstLegend,
    RightIntegralSecondLegend,
    LeftIntegralFirstLegend,
    LeftIntegralSecondLegend,
    FiniteIntegralLegend,
    DiscreteLegend,

    RegressionFormula,

    French,
    English,
    Spanish,
    German,
    Portuguese,

    FccId,

    /* Toolbox: commands */
    AbsCommand,
    RootCommand,
    LogCommand,
    DiffCommand,
    IntCommand,
    SumCommand,
    ProductCommand,
    ArgCommand,
    ReCommand,
    ImCommand,
    ConjCommand,
    BinomialCommand,
    PermuteCommand,
    GcdCommand,
    LcmCommand,
    RemCommand,
    QuoCommand,
    InverseCommand,
    DeterminantCommand,
    TransposeCommand,
    TraceCommand,
    DimensionCommand,
    SortCommand,
    InvSortCommand,
    MaxCommand,
    MinCommand,
    FloorCommand,
    FracCommand,
    CeilCommand,
    RoundCommand,
    CoshCommand,
    SinhCommand,
    TanhCommand,
    AcoshCommand,
    AsinhCommand,
    AtanhCommand,
    Prediction95Command,
    PredictionCommand,
    ConfidenceCommand,

    AbsCommandWithArg,
    RootCommandWithArg,
    LogCommandWithArg,
    DiffCommandWithArg,
    IntCommandWithArg,
    SumCommandWithArg,
    ProductCommandWithArg,
    ArgCommandWithArg,
    ReCommandWithArg,
    ImCommandWithArg,
    ConjCommandWithArg,
    BinomialCommandWithArg,
    PermuteCommandWithArg,
    GcdCommandWithArg,
    LcmCommandWithArg,
    RemCommandWithArg,
    QuoCommandWithArg,
    InverseCommandWithArg,
    DeterminantCommandWithArg,
    TransposeCommandWithArg,
    TraceCommandWithArg,
    DimensionCommandWithArg,
    SortCommandWithArg,
    InvSortCommandWithArg,
    MaxCommandWithArg,
    MinCommandWithArg,
    FloorCommandWithArg,
    FracCommandWithArg,
    CeilCommandWithArg,
    RoundCommandWithArg,
    CoshCommandWithArg,
    SinhCommandWithArg,
    TanhCommandWithArg,
    AcoshCommandWithArg,
    AsinhCommandWithArg,
    AtanhCommandWithArg,
    Prediction95CommandWithArg,
    PredictionCommandWithArg,
    ConfidenceCommandWithArg,
  };
  enum class Language : uint16_t {
    Default = 0,
    English = 1,
    French = 2,
    Spanish = 3,
    German = 4,
    Portuguese = 5
  };
}

#endif

