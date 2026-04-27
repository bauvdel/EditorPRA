class PRAEditorGameCache
{
    protected static ref PRAEditorGameCache s_Instance;
    protected DayZGame m_Game;
    protected UIManager m_UIManager;
    protected ScriptCallQueue m_GUICallQueue;
    protected ScriptCallQueue m_SystemCallQueue;
    protected Editor m_Editor;
    protected bool m_GameCached;
    protected bool m_UIManagerCached;
    protected bool m_GUICallQueueCached;
    protected bool m_SystemCallQueueCached;
    protected bool m_EditorCached;

    protected void PRAEditorGameCache()
    {
        // Initialize cache flags to false - cache starts empty
        m_GameCached = false;
        m_UIManagerCached = false;
        m_GUICallQueueCached = false;
        m_SystemCallQueueCached = false;
        m_EditorCached = false;
    }

    static PRAEditorGameCache GetInstance()
    {
        if (!s_Instance)
            s_Instance = new PRAEditorGameCache();
        return s_Instance;
    }

    static DayZGame GetCachedGame()
    {
        PRAEditorGameCache cache = GetInstance();
        if (!cache.m_GameCached || !cache.m_Game)
        {
            cache.m_Game = GetGame();
            cache.m_GameCached = true;
        }
        return cache.m_Game;
    }

    static UIManager GetCachedUIManager()
    {
        PRAEditorGameCache cache = GetInstance();
        if (!cache.m_UIManagerCached || !cache.m_UIManager)
        {
            DayZGame game = GetCachedGame();
            if (game)
            {
                cache.m_UIManager = game.GetUIManager();
                cache.m_UIManagerCached = true;
            }
        }
        return cache.m_UIManager;
    }

    static ScriptCallQueue GetCachedGUICallQueue()
    {
        PRAEditorGameCache cache = GetInstance();
        if (!cache.m_GUICallQueueCached || !cache.m_GUICallQueue)
        {
            DayZGame game = GetCachedGame();
            if (game)
            {
                cache.m_GUICallQueue = game.GetCallQueue(CALL_CATEGORY_GUI);
                cache.m_GUICallQueueCached = true;
            }
        }
        return cache.m_GUICallQueue;
    }

    static ScriptCallQueue GetCachedSystemCallQueue()
    {
        PRAEditorGameCache cache = GetInstance();
        if (!cache.m_SystemCallQueueCached || !cache.m_SystemCallQueue)
        {
            DayZGame game = GetCachedGame();
            if (game)
            {
                cache.m_SystemCallQueue = game.GetCallQueue(CALL_CATEGORY_SYSTEM);
                cache.m_SystemCallQueueCached = true;
            }
        }
        return cache.m_SystemCallQueue;
    }

    static Editor GetCachedEditor()
    {
        PRAEditorGameCache cache = GetInstance();
        if (!cache.m_EditorCached || !cache.m_Editor)
        {
            cache.m_Editor = GetEditor();
            cache.m_EditorCached = true;
        }
        return cache.m_Editor;
    }

    static float GetCachedTime()
    {
        DayZGame game = GetCachedGame();
        if (game)
            return game.GetTime() * 0.001; // Convert to seconds
        return 0;
    }

    static void InvalidateCache()
    {
        if (!s_Instance)
            return;

        s_Instance.m_Game = null;
        s_Instance.m_UIManager = null;
        s_Instance.m_GUICallQueue = null;
        s_Instance.m_SystemCallQueue = null;
        s_Instance.m_Editor = null;

        s_Instance.m_GameCached = false;
        s_Instance.m_UIManagerCached = false;
        s_Instance.m_GUICallQueueCached = false;
        s_Instance.m_SystemCallQueueCached = false;
        s_Instance.m_EditorCached = false;
    }

    static bool IsCacheValid()
    {
        PRAEditorGameCache cache = GetInstance();
        return cache.m_GameCached && cache.m_Game;
    }

    static void RefreshCache()
    {
        InvalidateCache();
        GetCachedGame();
        GetCachedUIManager();
        GetCachedGUICallQueue();
        GetCachedSystemCallQueue();
    }

    static void Cleanup()
    {
        if (s_Instance)
        {
            s_Instance.InvalidateCache();
            s_Instance = null;
        }
    }
}
