// Centralized configuration for PRA Editor
class PRAEditorSettings
{
    // Step size for incremental scaling operations
    protected static const float DEFAULT_STEP_SIZE = 1;

    // Default PRA box dimensions (Length, Height, Width)
    protected static const vector DEFAULT_BOX_SIZE = "10 10 10";

    // Minimum allowed dimension for any axis
    protected static const float MIN_DIMENSION = 0.001;

    // Distance tolerance for position matching during import/export
    protected static const float POSITION_TOLERANCE = 0.1;

    // Timer intervals
    protected static const float SYNC_TIMER_INTERVAL = 0.05;       // 50ms update cycle
    protected static const float UNDO_CACHE_TTL = 90;              // 90 second undo history

    // UI notification delays
    protected static const int DIALOG_CLOSE_DELAY = 100;           // milliseconds

    // Polygon line update rate
    protected static const float POLYGON_UPDATE_INTERVAL = 0.2;    // 5Hz for line updates

    static float GetStepSize()
    {
        return DEFAULT_STEP_SIZE;
    }

    static vector GetDefaultBoxSize()
    {
        return DEFAULT_BOX_SIZE;
    }

    static float GetMinDimension()
    {
        return MIN_DIMENSION;
    }

    static float GetPositionTolerance()
    {
        return POSITION_TOLERANCE;
    }

    static float GetSyncTimerInterval()
    {
        return SYNC_TIMER_INTERVAL;
    }

    static float GetUndoCacheTTL()
    {
        return UNDO_CACHE_TTL;
    }

    static int GetDialogCloseDelay()
    {
        return DIALOG_CLOSE_DELAY;
    }

    static float GetPolygonUpdateInterval()
    {
        return POLYGON_UPDATE_INTERVAL;
    }

    static vector ClampSize(vector size)
    {
        vector result = size;
        if (result[0] <= MIN_DIMENSION) result[0] = MIN_DIMENSION;
        if (result[1] <= MIN_DIMENSION) result[1] = MIN_DIMENSION;
        if (result[2] <= MIN_DIMENSION) result[2] = MIN_DIMENSION;
        return result;
    }
}
