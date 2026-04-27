class PRAPolygonVertex : Building
{
	private string m_AreaGroupName = "UnnamedArea";
	protected int m_VertexIndex = 0;
	protected ref Timer m_SyncTimer;

	// Reference to the polygon manager (set by manager)
	protected PRAPolygonManager m_Manager;

	void PRAPolygonVertex()
	{
		SetEventMask(EntityEvent.INIT);

		// Adjust to terrain height
		vector pos = GetPosition();
		pos[1] = GetGame().SurfaceY(pos[0], pos[2]);
		SetPosition(pos);

		// Start position sync timer (10Hz)
		if (!m_SyncTimer) {
			m_SyncTimer = new Timer();
			m_SyncTimer.Run(0.1, this, "SyncPosition", null, true);
		}

		// Inherit area name from existing PRA objects FIRST
		InheritAreaNameFromScene();

		// Then register with polygon manager (after area name is set)
		PRAPolygonManager manager = PRAPolygonManager.GetInstance();
		if (manager) {
			manager.RegisterVertex(this);

			// Debug: Print registration
			Print(string.Format("[PRA Debug] Registered vertex at %1 with area: %2", GetPosition().ToString(), m_AreaGroupName));
		}
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

	void ~PRAPolygonVertex()
	{
		if (m_SyncTimer) {
			m_SyncTimer.Stop();
		}

		// Notify manager that this vertex is being deleted
		if (m_Manager) {
			m_Manager.OnVertexDeleted(this);
		}
	}

	// Area grouping
	void SetAreaGroup(string areaName)
	{
		if (areaName == "") areaName = "UnnamedArea";
		m_AreaGroupName = areaName;

		// Notify manager of area change
		if (m_Manager) {
			m_Manager.OnVertexAreaChanged(this);
		}

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
			PRASafePosition2D safe2D = PRASafePosition2D.Cast(w);
			if (safe2D) {
				safe2D.SetAreaGroupDirect(areaName);
				continue;
			}

			// Sync PRAPolygonVertex
			PRAPolygonVertex otherVertex = PRAPolygonVertex.Cast(w);
			if (otherVertex) {
				otherVertex.SetAreaGroupDirect(areaName);
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

	// Vertex ordering
	void SetVertexIndex(int index)
	{
		m_VertexIndex = index;
	}

	int GetVertexIndex()
	{
		return m_VertexIndex;
	}

	// Manager reference
	void SetManager(PRAPolygonManager manager)
	{
		m_Manager = manager;
	}

	PRAPolygonManager GetManager()
	{
		return m_Manager;
	}

	// Position synchronization
	protected void SyncPosition()
	{
		// Notify manager that position changed (for line updates)
		if (m_Manager) {
			m_Manager.OnVertexMoved(this);
		}
	}

	// Get position for export
	vector GetPosition2D()
	{
		vector pos = GetPosition();
		return Vector(pos[0], 0, pos[2]); // X and Z only, Y zeroed
	}

	// Serialization support
	override void EOnInit(IEntity other, int extra)
	{
		super.EOnInit(other, extra);

		// Register with global polygon manager
		PRAPolygonManager globalManager = PRAPolygonManager.GetInstance();
		if (globalManager) {
			globalManager.RegisterVertex(this);
		}
	}
}
