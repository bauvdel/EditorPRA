modded class EditorExportMenu
{
    void EditorExportMenu()
    {
        // Add PRA export button
        AddMenuButton(m_Editor.CommandManager[ExportPRAToJSON]);
    }
}