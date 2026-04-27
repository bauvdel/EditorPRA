class EditorImportFromPRA: EditorImportCommandBase
{
	override typename GetFileType()
	{
		return EditorPRAFile;
	}

	override string GetName()
	{
		return "Import from PRA (*.json)";
	}
}
