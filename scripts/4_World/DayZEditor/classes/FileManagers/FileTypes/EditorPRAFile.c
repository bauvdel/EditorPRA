class EditorPRAFile : EditorFileType
{
	override void Export(EditorSaveData data, string file, ExportSettings settings, eDialogExtraSetting dialog_setting)
	{
		// Use vanilla single-area format
		PRAAreaExport singleArea = new PRAAreaExport("");

		if (!ValidateExportPreconditions(file, singleArea))
			return;

		Editor editor = GetEditor();

		// Auto-sync all area names before export
		editor.PRA_SyncAllAreaNames();

		EditorObjectMap allObjects = editor.GetPlacedObjects();

		if (!allObjects) {
			FinalizeExportSingleArea(file, singleArea);
			return;
		}

		ProcessPRAObjectsSingleArea(data, allObjects, singleArea);
		FinalizeExportSingleArea(file, singleArea);
	}

	// Validate export preconditions
	protected bool ValidateExportPreconditions(string file, PRAAreaExport area)
	{
		Editor editor = GetEditor();
		if (!editor) {
			// TODO: Show error dialog
			return false;
		}
		return true;
	}

	// Process PRA objects for single-area export (vanilla format)
	protected void ProcessPRAObjectsSingleArea(EditorSaveData data, EditorObjectMap allObjects, PRAAreaExport area)
	{
		// Track processed object IDs to avoid duplicates
		map<int, bool> processedIds = new map<int, bool>();

		// Get first object's area name (all should be same for single-area export)
		string areaName = "UnnamedArea";
		bool areaNameSet = false;

		foreach (EditorObjectData obj_data : data.EditorObjects)
		{
			if (!obj_data) continue;

			// Process PRABoxObject
			if (obj_data.Type == "PRABoxObject")
			{
				PRABoxObject box = FindMatchingPRABox(obj_data, allObjects);
				if (!box) continue;

				EditorObject eo = FindEditorObjectForWorldObject(box, allObjects);
				if (!eo) continue;

				int objId = eo.GetID();
				if (processedIds.Contains(objId)) continue;
				processedIds.Insert(objId, true);

				if (!areaNameSet) {
					areaName = box.GetAreaGroup();
					if (areaName == "") areaName = "UnnamedArea";
					area.areaName = areaName;
					areaNameSet = true;
				}

				vector size = box.GetSize();
				vector rotation = box.GetRotationEuler();
				vector position = obj_data.Position;

				area.AddBox(size, rotation, position);
			}
			// Process PRASafePosition3D
			else if (obj_data.Type == "PRASafePosition3D")
			{
				PRASafePosition3D safe3D = FindMatchingSafePosition3D(obj_data, allObjects);
				if (!safe3D) continue;

				EditorObject eo3D = FindEditorObjectForWorldObject(safe3D, allObjects);
				if (!eo3D) continue;

				int objId3D = eo3D.GetID();
				if (processedIds.Contains(objId3D)) continue;
				processedIds.Insert(objId3D, true);

				if (!areaNameSet) {
					areaName = safe3D.GetAreaGroup();
					if (areaName == "") areaName = "UnnamedArea";
					area.areaName = areaName;
					areaNameSet = true;
				}

				vector pos3D = obj_data.Position;
				area.AddSafePosition3D(pos3D);
			}
			// Process PRASafePosition2D
			else if (obj_data.Type == "PRASafePosition2D")
			{
				PRASafePosition2D safe2D = FindMatchingSafePosition2D(obj_data, allObjects);
				if (!safe2D) continue;

				EditorObject eo2D = FindEditorObjectForWorldObject(safe2D, allObjects);
				if (!eo2D) continue;

				int objId2D = eo2D.GetID();
				if (processedIds.Contains(objId2D)) continue;
				processedIds.Insert(objId2D, true);

				if (!areaNameSet) {
					areaName = safe2D.GetAreaGroup();
					if (areaName == "") areaName = "UnnamedArea";
					area.areaName = areaName;
					areaNameSet = true;
				}

				vector pos2D = obj_data.Position;
				area.AddSafePosition2D(pos2D);
			}
		}

		// Process polygons from PRAPolygonManager
		PRAPolygonManager polygonManager = PRAPolygonManager.GetInstance();
		if (polygonManager) {
			array<string> polygonAreaNames = polygonManager.GetAreaNames();

			foreach (string polyAreaName : polygonAreaNames) {
				if (!areaNameSet) {
					area.areaName = polyAreaName;
					areaNameSet = true;
				}

				array<PRAPolygonVertex> vertices = polygonManager.GetVerticesForArea(polyAreaName);

				// Add polygon if valid (at least 3 vertices)
				if (vertices && vertices.Count() >= 3) {
					area.AddPolygon(vertices);
				}
			}
		}
	}

	// Finalize single-area export (vanilla format)
	protected void FinalizeExportSingleArea(string file, PRAAreaExport area)
	{
		JsonFileLoader<PRAAreaExport>.JsonSaveFile(file, area);

		Editor editor = GetEditor();
		if (editor) {
			editor.GetEditorHud().CreateNotification("[PRA] Export complete");
		}
	}

	// Process PRA objects for export (grouped by area name)
	protected void ProcessPRAObjects(EditorSaveData data, EditorObjectMap allObjects, PRAExportRoot root)
	{
		// Group objects by area name
		map<string, ref PRAAreaExport> areaMap = new map<string, ref PRAAreaExport>();

		// Track processed object IDs to avoid duplicates
		map<int, bool> processedIds = new map<int, bool>();

		foreach (EditorObjectData obj_data : data.EditorObjects)
		{
			if (!obj_data) continue;

			string areaName = "";
			PRAAreaExport area;
			PRAAreaExport newArea;

			// Process PRABoxObject
			if (obj_data.Type == "PRABoxObject")
			{
				PRABoxObject box = FindMatchingPRABox(obj_data, allObjects);
				if (!box) continue;

				// Get the EditorObject ID to check for duplicates
				EditorObject eo = FindEditorObjectForWorldObject(box, allObjects);
				if (!eo) continue;

				int objId = eo.GetID();
				if (processedIds.Contains(objId)) continue; // Skip duplicate
				processedIds.Insert(objId, true);

				areaName = box.GetAreaGroup();
				if (areaName == "") areaName = "UnnamedArea";

				// Create area if doesn't exist
				if (!areaMap.Contains(areaName)) {
					newArea = new PRAAreaExport(areaName);
					areaMap.Insert(areaName, newArea);
				}

				area = areaMap.Get(areaName);
				vector size = box.GetSize();
				vector rotation = box.GetRotationEuler();
				vector position = obj_data.Position;

				area.AddBox(size, rotation, position);
			}
			// Process PRASafePosition3D
			else if (obj_data.Type == "PRASafePosition3D")
			{
				PRASafePosition3D safe3D = FindMatchingSafePosition3D(obj_data, allObjects);
				if (!safe3D) continue;

				EditorObject eo3D = FindEditorObjectForWorldObject(safe3D, allObjects);
				if (!eo3D) continue;

				int objId3D = eo3D.GetID();
				if (processedIds.Contains(objId3D)) continue;
				processedIds.Insert(objId3D, true);

				areaName = safe3D.GetAreaGroup();
				if (areaName == "") areaName = "UnnamedArea";

				// Create area if doesn't exist
				if (!areaMap.Contains(areaName)) {
					newArea = new PRAAreaExport(areaName);
					areaMap.Insert(areaName, newArea);
				}

				area = areaMap.Get(areaName);
				vector pos3D = obj_data.Position;
				area.AddSafePosition3D(pos3D);
			}
			// Process PRASafePosition2D
			else if (obj_data.Type == "PRASafePosition2D")
			{
				PRASafePosition2D safe2D = FindMatchingSafePosition2D(obj_data, allObjects);
				if (!safe2D) continue;

				EditorObject eo2D = FindEditorObjectForWorldObject(safe2D, allObjects);
				if (!eo2D) continue;

				int objId2D = eo2D.GetID();
				if (processedIds.Contains(objId2D)) continue;
				processedIds.Insert(objId2D, true);

				areaName = safe2D.GetAreaGroup();
				if (areaName == "") areaName = "UnnamedArea";

				// Create area if doesn't exist
				if (!areaMap.Contains(areaName)) {
					newArea = new PRAAreaExport(areaName);
					areaMap.Insert(areaName, newArea);
				}

				area = areaMap.Get(areaName);
				vector pos2D = obj_data.Position;
				// For 2D, only export X and Z
				area.AddSafePosition2D(pos2D);
			}
		}

		// Process polygons from PRAPolygonManager
		PRAPolygonManager polygonManager = PRAPolygonManager.GetInstance();
		if (polygonManager) {
			array<string> polygonAreaNames = polygonManager.GetAreaNames();

			foreach (string polyAreaName : polygonAreaNames) {
				// Create area if doesn't exist
				if (!areaMap.Contains(polyAreaName)) {
					newArea = new PRAAreaExport(polyAreaName);
					areaMap.Insert(polyAreaName, newArea);
				}

				area = areaMap.Get(polyAreaName);
				array<PRAPolygonVertex> vertices = polygonManager.GetVerticesForArea(polyAreaName);

				// Add polygon if valid (at least 3 vertices)
				if (vertices && vertices.Count() >= 3) {
					area.AddPolygon(vertices);
				}
			}
		}

		// Convert map to array
		foreach (string exportAreaName, PRAAreaExport areaExport : areaMap)
		{
			root.AddArea(areaExport);
		}
	}

	// Find PRA box object by position matching
	protected PRABoxObject FindMatchingPRABox(EditorObjectData obj_data, EditorObjectMap allObjects)
	{
		float tolerance = 0.1; // Position matching tolerance in meters

		foreach (int id, EditorObject placed_obj : allObjects) {
			if (!placed_obj || !placed_obj.GetWorldObject()) continue;

			vector objPos = placed_obj.GetPosition();
			if (vector.Distance(objPos, obj_data.Position) < tolerance) {
				Object worldObj = placed_obj.GetWorldObject();
				if (worldObj && worldObj.IsKindOf("PRABoxObject")) {
					PRABoxObject box = PRABoxObject.Cast(worldObj);
					if (box) return box;
				}
			}
		}
		return null;
	}

	// Find PRASafePosition3D object by position matching
	protected PRASafePosition3D FindMatchingSafePosition3D(EditorObjectData obj_data, EditorObjectMap allObjects)
	{
		float tolerance = 0.1;

		foreach (int id, EditorObject placed_obj : allObjects) {
			if (!placed_obj || !placed_obj.GetWorldObject()) continue;

			vector objPos = placed_obj.GetPosition();
			if (vector.Distance(objPos, obj_data.Position) < tolerance) {
				Object worldObj = placed_obj.GetWorldObject();
				if (worldObj && worldObj.IsKindOf("PRASafePosition3D")) {
					PRASafePosition3D safe3D = PRASafePosition3D.Cast(worldObj);
					if (safe3D) return safe3D;
				}
			}
		}
		return null;
	}

	// Find PRASafePosition2D object by position matching
	protected PRASafePosition2D FindMatchingSafePosition2D(EditorObjectData obj_data, EditorObjectMap allObjects)
	{
		float tolerance = 0.1;

		foreach (int id, EditorObject placed_obj : allObjects) {
			if (!placed_obj || !placed_obj.GetWorldObject()) continue;

			vector objPos = placed_obj.GetPosition();
			if (vector.Distance(objPos, obj_data.Position) < tolerance) {
				Object worldObj = placed_obj.GetWorldObject();
				if (worldObj && worldObj.IsKindOf("PRASafePosition2D")) {
					PRASafePosition2D safe2D = PRASafePosition2D.Cast(worldObj);
					if (safe2D) return safe2D;
				}
			}
		}
		return null;
	}

	// Find EditorObject for a given world object
	protected EditorObject FindEditorObjectForWorldObject(Object worldObj, EditorObjectMap allObjects)
	{
		if (!worldObj) return null;

		foreach (int id, EditorObject eo : allObjects) {
			if (!eo) continue;
			if (eo.GetWorldObject() == worldObj) {
				return eo;
			}
		}
		return null;
	}

	// Finalize export operation
	protected void FinalizeExport(string file, PRAExportRoot root)
	{
		JsonFileLoader<PRAExportRoot>.JsonSaveFile(file, root);

		Editor editor = GetEditor();
		if (editor) {
			editor.GetEditorHud().CreateNotification("[PRA] Export complete");
		}
	}

	// Import PRA objects from JSON (vanilla single-area format)
	override EditorSaveData Import(string file, ImportSettings settings)
	{
		EditorSaveData save_data = new EditorSaveData();

		// Load as single-area format (vanilla DayZ format)
		PRAAreaExport area = new PRAAreaExport("");
		JsonFileLoader<PRAAreaExport>.JsonLoadFile(file, area);

		if (!area || (!area.PRABoxes && !area.safePositions3D && !area.safePositions2D && !area.PRAPolygons)) {
			// No valid data found
			return save_data;
		}

		ProcessAreaImport(area, save_data);

		// Start deferred property application
		PRA_PostImportApplier.Start();

		return save_data;
	}

	// Process a single area's import data
	protected void ProcessAreaImport(PRAAreaExport area, EditorSaveData save_data)
	{
		if (!area) return;

		string areaName = area.areaName;
		if (areaName == "") areaName = "UnnamedArea";

		// Import PRABoxes
		if (area.PRABoxes)
		{
			foreach (auto boxData : area.PRABoxes)
			{
				if (boxData.Count() != 3) continue;

				array<float> sizeArr = boxData.Get(0);
				array<float> rotArr = boxData.Get(1);
				array<float> posArr = boxData.Get(2);

				if (!sizeArr || sizeArr.Count() != 3 || !rotArr || rotArr.Count() != 3 || !posArr || posArr.Count() != 3) {
					continue;
				}

				vector size = Vector(sizeArr[0], sizeArr[1], sizeArr[2]);
				vector rotation = Vector(rotArr[0], rotArr[1], rotArr[2]);
				vector position = Vector(posArr[0], posArr[1], posArr[2]);

				// Queue for deferred application
				PRABoxApplyRec boxRec = new PRABoxApplyRec();
				boxRec.Pos = position;
				boxRec.Size = size;
				boxRec.AreaGroupName = areaName;
				g_PRABox_ToApply.Insert(boxRec);

				// Create editor object
				EditorObjectData boxObjData = EditorObjectData.Create("PRABoxObject", position, rotation, 1, EFE_DEFAULT);
				save_data.EditorObjects.Insert(boxObjData);
			}
		}

		// Import safePositions3D
		if (area.safePositions3D) {
			foreach (auto pos3DArr : area.safePositions3D)
			{
				if (pos3DArr.Count() != 3) continue;

				vector position3D = Vector(pos3DArr[0], pos3DArr[1], pos3DArr[2]);
				vector orientation3D = Vector(0, 0, 0);

				// Queue for deferred application
				PRASafePosition3DApplyRec safe3DRec = new PRASafePosition3DApplyRec();
				safe3DRec.Pos = position3D;
				safe3DRec.AreaGroupName = areaName;
				g_PRASafe3D_ToApply.Insert(safe3DRec);

				// Create editor object
				EditorObjectData safe3DData = EditorObjectData.Create("PRASafePosition3D", position3D, orientation3D, 1, EFE_DEFAULT);
				save_data.EditorObjects.Insert(safe3DData);
			}
		}

		// Import safePositions2D
		if (area.safePositions2D) {
			foreach (auto pos2DArr : area.safePositions2D)
			{
				if (pos2DArr.Count() != 2) continue;

				// For 2D, get terrain height for Y coordinate
				float x = pos2DArr[0];
				float z = pos2DArr[1];
				float y = GetGame().SurfaceY(x, z);
				vector position2D = Vector(x, y, z);
				vector orientation2D = Vector(0, 0, 0);

				// Queue for deferred application
				PRASafePosition2DApplyRec safe2DRec = new PRASafePosition2DApplyRec();
				safe2DRec.Pos = position2D;
				safe2DRec.AreaGroupName = areaName;
				g_PRASafe2D_ToApply.Insert(safe2DRec);

				// Create editor object
				EditorObjectData safe2DData = EditorObjectData.Create("PRASafePosition2D", position2D, orientation2D, 1, EFE_DEFAULT);
				save_data.EditorObjects.Insert(safe2DData);
			}
		}

		// Import PRAPolygons
		if (area.PRAPolygons) {
			foreach (auto polygonData : area.PRAPolygons)
			{
				if (polygonData.Count() < 3) continue; // Need at least 3 vertices

				PRAPolygonApplyRec polyRec = new PRAPolygonApplyRec();
				polyRec.AreaGroupName = areaName;
				polyRec.Vertices = new array<vector>();

				// Create vertices
				foreach (auto vertexArr : polygonData)
				{
					if (vertexArr.Count() != 2) continue;

					// Polygon vertices are 2D - get terrain height for Y coordinate
					float vx = vertexArr[0];
					float vz = vertexArr[1];
					float vy = GetGame().SurfaceY(vx, vz);
					vector vertexPos = Vector(vx, vy, vz);
					vector orientation = Vector(0, 0, 0);

					polyRec.Vertices.Insert(vertexPos);

					// Create editor object
					EditorObjectData vertexData = EditorObjectData.Create("PRAPolygonVertex", vertexPos, orientation, 1, EFE_DEFAULT);
					save_data.EditorObjects.Insert(vertexData);
				}

				// Queue polygon for deferred application
				if (polyRec.Vertices.Count() >= 3) {
					g_PRAPolygon_ToApply.Insert(polyRec);
				}
			}
		}

		// Start deferred property application
		PRA_PostImportApplier.Start();
	}

	override string GetExtension()
	{
		return ".json";
	}

	override void GetValidExtensions(notnull inout array<ref Param2<string, string>> valid_extensions)
	{
		super.GetValidExtensions(valid_extensions);
		valid_extensions.Insert(new Param2<string, string>("PRA Export", "*.json"));
	}
}
