// - Add PRA export/import to File Menu in Editor
modded class EditorCommandManager
{
	override void Init()
	{
        // Register PRA export and import commands
        RegisterCommand(ExportPRAToJSON);
        RegisterCommand(EditorImportFromPRA);

        super.Init();
    }
}