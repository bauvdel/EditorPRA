class ExportPRAToJSON: EditorExportCommandBase
{
	override typename GetFileType()
	{
		return EditorPRAFile;
	}

	override string GetName()
	{
		return "Export to PRA (*.json)";
	}
}
