class PRASafePosition2D : Building
{
	private string m_AreaGroupName = "UnnamedArea";
	protected ref Timer m_SyncTimer;
	protected vector m_LastPosition;

	// Constructor
	void PRASafePosition2D()
	{
		InitializeDefaultSettings();
		InitializeSyncTimer();
	}

	// Destructor
	void ~PRASafePosition2D()
	{
		if (m_SyncTimer) m_SyncTimer.Stop();
	}


	// Initialize with default settings
	protected void InitializeDefaultSettings()
	{
		// Adjust to terrain height
		vector pos = GetPosition();
		pos[1] = GetGame().SurfaceY(pos[0], pos[2]);
		SetPosition(pos);

		m_LastPosition = pos;

		// Inherit area name from existing PRA objects
		InheritAreaNameFromScene();
	}

	// Inherit area name from existing PRA objects when first placed
	protected void InheritAreaNameFromScene()
	{
		Editor editor = GetEditor();
		if (!editor) return;

		EditorObjectMap placed = editor.GetPlacedObjects();
		if (!placed || placed.Count() == 0) return;

		// Find first existing PRA object with non-default area name
		string existingAreaName = "";

		foreach (int id, EditorObject eo : placed)
		{
			if (!eo) continue;
			Object w = eo.GetWorldObject();
			if (!w || w == this) continue; // Skip self

			// Check PRABoxObject
			PRABoxObject box = PRABoxObject.Cast(w);
			if (box) {
				existingAreaName = box.GetAreaGroup();
				if (existingAreaName != "" && existingAreaName != "UnnamedArea") {
					break;
				}
			}

			// Check PRASafePosition3D
			PRASafePosition3D safe3D = PRASafePosition3D.Cast(w);
			if (safe3D) {
				existingAreaName = safe3D.GetAreaGroup();
				if (existingAreaName != "" && existingAreaName != "UnnamedArea") {
					break;
				}
			}

			// Check PRASafePosition2D
			PRASafePosition2D safe2D = PRASafePosition2D.Cast(w);
			if (safe2D) {
				existingAreaName = safe2D.GetAreaGroup();
				if (existingAreaName != "" && existingAreaName != "UnnamedArea") {
					break;
				}
			}

			// Check PRAPolygonVertex
			PRAPolygonVertex vertex = PRAPolygonVertex.Cast(w);
			if (vertex) {
				existingAreaName = vertex.GetAreaGroup();
				if (existingAreaName != "" && existingAreaName != "UnnamedArea") {
					break;
				}
			}
		}

		// If we found an area name, use it (but don't trigger sync notification)
		if (existingAreaName != "" && existingAreaName != "UnnamedArea") {
			m_AreaGroupName = existingAreaName;
		}
	}

	/**
	 * @brief Initialize the synchronization timer
	 */
	protected void InitializeSyncTimer()
	{
		m_SyncTimer = new Timer(CALL_CATEGORY_SYSTEM);
		m_SyncTimer.Run(0.1, this, "UpdatePosition", null, true); // 10Hz updates
	}

	// ===== Area Grouping =====
	void SetAreaGroup(string areaName)
	{
		if (areaName == "") areaName = "UnnamedArea";
		m_AreaGroupName = areaName;

		// Auto-sync all other PRA objects to the same area name
		SyncAllPRAObjectsToArea(areaName);
	}

	// Direct setter for sync operations (bypasses auto-sync to avoid recursion)
	void SetAreaGroupDirect(string areaName)
	{
		if (areaName == "") areaName = "UnnamedArea";
		m_AreaGroupName = areaName;
	}

	// Sync all PRA objects in the scene to the given area name
	protected void SyncAllPRAObjectsToArea(string areaName)
	{
		// Get editor instance
		Editor editor = GetEditor();
		if (!editor) return;

		EditorObjectMap placed = editor.GetPlacedObjects();
		if (!placed || placed.Count() == 0) return;

		// Sync all PRA objects
		foreach (int id, EditorObject eo : placed)
		{
			if (!eo) continue;
			Object w = eo.GetWorldObject();
			if (!w) continue;

			// Skip self
			if (w == this) continue;

			// Sync PRABoxObject
			PRABoxObject box = PRABoxObject.Cast(w);
			if (box) {
				box.SetAreaGroupDirect(areaName); // Direct set to avoid recursion
				continue;
			}

			// Sync PRASafePosition3D
			PRASafePosition3D safe3D = PRASafePosition3D.Cast(w);
			if (safe3D) {
				safe3D.SetAreaGroupDirect(areaName);
				continue;
			}

			// Sync PRASafePosition2D
			PRASafePosition2D otherSafe2D = PRASafePosition2D.Cast(w);
			if (otherSafe2D) {
				otherSafe2D.SetAreaGroupDirect(areaName);
				continue;
			}

			// Sync PRAPolygonVertex
			PRAPolygonVertex vertex = PRAPolygonVertex.Cast(w);
			if (vertex) {
				vertex.SetAreaGroupDirect(areaName);
				continue;
			}
		}

		// Show notification
		editor.GetEditorHud().CreateNotification(string.Format("[PRA] All objects synced to area: %1", areaName));
	}

	string GetAreaGroup()
	{
		return m_AreaGroupName;
	}

	// ===== Position Management =====
	vector GetPosition2D()
	{
		vector pos = GetPosition();
		// Return [X, Z] - Y will be auto-calculated by game via g_Game.SurfaceY()
		return Vector(pos[0], 0, pos[2]);
	}

	protected void UpdatePosition()
	{
		vector currentPos = GetPosition();

		// Check if position changed
		if (vector.Distance(currentPos, m_LastPosition) > 0.01)
		{
			m_LastPosition = currentPos;
			OnPositionChanged();
		}
	}

	protected void OnPositionChanged()
	{
		// Hook for future extensions (e.g., updating visualizations)
	}
}
