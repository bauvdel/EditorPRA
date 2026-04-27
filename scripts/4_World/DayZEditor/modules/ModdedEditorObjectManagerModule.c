modded class EditorObjectManagerModule
{
    void EditorObjectManagerModule(Editor editor)
    {
        EditorPlaceableItem epi;

        // Enable PRABoxObject for console mode
        if (m_PlaceableObjectsByType && m_PlaceableObjectsByType.Find("PRABoxObject", epi) && epi) {
            epi.ConsoleFriendly = 1;
        }

        // Enable PRASafePosition3D for console mode
        if (m_PlaceableObjectsByType && m_PlaceableObjectsByType.Find("PRASafePosition3D", epi) && epi) {
            epi.ConsoleFriendly = 1;
        }

        // Enable PRASafePosition2D for console mode
        if (m_PlaceableObjectsByType && m_PlaceableObjectsByType.Find("PRASafePosition2D", epi) && epi) {
            epi.ConsoleFriendly = 1;
        }

        // Enable PRAPolygonVertex for console mode
        if (m_PlaceableObjectsByType && m_PlaceableObjectsByType.Find("PRAPolygonVertex", epi) && epi) {
            epi.ConsoleFriendly = 1;
        }
    }
}
