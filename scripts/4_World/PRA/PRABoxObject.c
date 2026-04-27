class PRABoxObject : Building
{
	protected vector m_Size;
	private string m_AreaGroupName = "UnnamedArea";
	protected ref Timer m_SyncTimer;
	protected vector m_LastPosePos;
	protected vector m_LastPoseOri;

	// Constructor
	void PRABoxObject()
	{
		InitializeDefaultSettings();
		InitializeSyncTimer();
	}

	// Destructor
	void ~PRABoxObject()
	{
		if (m_SyncTimer) m_SyncTimer.Stop();
	}

	/**
	 * @brief Initialize box with default settings
	 */
	protected void InitializeDefaultSettings()
	{
		m_Size = Vector(1, 1, 1);
		ApplySizeTransform();

		m_LastPosePos = GetPosition();
		m_LastPoseOri = GetOrientation();

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
		m_SyncTimer.Run(0.1, this, "UpdatePose", null, true); // 10Hz updates
	}

	// ===== Size Management =====
	void SetSize(vector sizeMeters)
	{
		// Validate and sanitize size (minimum 0.001m per dimension)
		float minDim = PRAEditorSettings.GetMinDimension();
		if (sizeMeters[0] < minDim) sizeMeters[0] = minDim;
		if (sizeMeters[1] < minDim) sizeMeters[1] = minDim;
		if (sizeMeters[2] < minDim) sizeMeters[2] = minDim;

		m_Size = sizeMeters;
		ApplySizeTransform();
	}

	void TrigSize(float dx, float dy, float dz)
	{
		float minDim = PRAEditorSettings.GetMinDimension();
		m_Size[0] = Math.Max(m_Size[0] + dx, minDim);
		m_Size[1] = Math.Max(m_Size[1] + dy, minDim);
		m_Size[2] = Math.Max(m_Size[2] + dz, minDim);
		ApplySizeTransform();
	}

	void SetTransformData()
	{
		vector m[4];
		GetTransform(m);
		m_Size = Vector(m[0].Length(), m[1].Length(), m[2].Length());
	}

	vector GetSize()
	{
		return m_Size;
	}

	protected void ApplySizeTransform()
	{
		vector T[4];
		GetTransform(T);

		vector ax0 = T[0].Normalized();
		vector ax1 = T[1].Normalized();
		vector ax2 = T[2].Normalized();

		T[0] = ax0 * m_Size[0];
		T[1] = ax1 * m_Size[1];
		T[2] = ax2 * m_Size[2];

		SetTransform(T);
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
			PRABoxObject otherBox = PRABoxObject.Cast(w);
			if (otherBox) {
				otherBox.SetAreaGroupDirect(areaName); // Direct set to avoid recursion
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

	// ===== Position/Rotation Sync =====
	protected void UpdatePose()
	{
		vector currentPos = GetPosition();
		vector currentOri = GetOrientation();

		// Check if position or orientation changed
		if (vector.Distance(currentPos, m_LastPosePos) > 0.01 || vector.Distance(currentOri, m_LastPoseOri) > 0.01) {
			m_LastPosePos = currentPos;
			m_LastPoseOri = currentOri;
			OnPoseChanged();
		}
	}

	protected void OnPoseChanged()
	{
		// Hook for future extensions (e.g., updating connected objects)
	}

	// ===== Rotation Helpers =====
	vector GetRotationEuler()
	{
		vector orient = GetOrientation();
		// DayZ uses yaw-pitch-roll order
		return Vector(orient[0], orient[1], orient[2]);
	}

	void SetRotationEuler(vector euler)
	{
		SetOrientation(euler);
		m_LastPoseOri = euler;
	}
}

// ===== IMPORT SYSTEM =====
// Deferred property application for JSON imports

class PRABoxApplyRec
{
	vector Pos;
	vector Size;
	string AreaGroupName;
}

class PRASafePosition3DApplyRec
{
	vector Pos;
	string AreaGroupName;
}

class PRASafePosition2DApplyRec
{
	vector Pos;
	string AreaGroupName;
}

class PRAPolygonApplyRec
{
	string AreaGroupName;
	ref array<vector> Vertices;
}

ref array<ref PRABoxApplyRec> g_PRABox_ToApply = new array<ref PRABoxApplyRec>();
ref array<ref PRASafePosition3DApplyRec> g_PRASafe3D_ToApply = new array<ref PRASafePosition3DApplyRec>();
ref array<ref PRASafePosition2DApplyRec> g_PRASafe2D_ToApply = new array<ref PRASafePosition2DApplyRec>();
ref array<ref PRAPolygonApplyRec> g_PRAPolygon_ToApply = new array<ref PRAPolygonApplyRec>();

class PRA_PostImportApplier
{
	static int s_Attempts = 20;

	static void Start()
	{
		s_Attempts = 20;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PRA_PostImportApplier.Tick, 200, false);
	}

	static void Tick()
	{
		// Apply PRABoxObject properties
		for (int i = g_PRABox_ToApply.Count() - 1; i >= 0; i--)
		{
			PRABoxApplyRec boxRec = g_PRABox_ToApply[i];
			PRABoxObject box = FindNearestPRABox(boxRec.Pos, 3);
			if (!box) continue;

			box.SetSize(boxRec.Size);
			if (boxRec.AreaGroupName != "") box.SetAreaGroup(boxRec.AreaGroupName);

			g_PRABox_ToApply.Remove(i);
		}

		// Apply PRASafePosition3D properties
		for (int j = g_PRASafe3D_ToApply.Count() - 1; j >= 0; j--)
		{
			PRASafePosition3DApplyRec safe3DRec = g_PRASafe3D_ToApply[j];
			PRASafePosition3D safe3D = FindNearestSafePosition3D(safe3DRec.Pos, 3);
			if (!safe3D) continue;

			if (safe3DRec.AreaGroupName != "") safe3D.SetAreaGroup(safe3DRec.AreaGroupName);

			g_PRASafe3D_ToApply.Remove(j);
		}

		// Apply PRASafePosition2D properties
		for (int k = g_PRASafe2D_ToApply.Count() - 1; k >= 0; k--)
		{
			PRASafePosition2DApplyRec safe2DRec = g_PRASafe2D_ToApply[k];
			PRASafePosition2D safe2D = FindNearestSafePosition2D(safe2DRec.Pos, 3);
			if (!safe2D) continue;

			if (safe2DRec.AreaGroupName != "") safe2D.SetAreaGroup(safe2DRec.AreaGroupName);

			g_PRASafe2D_ToApply.Remove(k);
		}

		// Apply PRAPolygonVertex properties
		PRAPolygonManager polygonManager = PRAPolygonManager.GetInstance();
		for (int m = g_PRAPolygon_ToApply.Count() - 1; m >= 0; m--)
		{
			PRAPolygonApplyRec polyRec = g_PRAPolygon_ToApply[m];
			if (!polyRec.Vertices || polyRec.Vertices.Count() < 3) {
				g_PRAPolygon_ToApply.Remove(m);
				continue;
			}

			// Try to find vertices near the recorded positions
			array<PRAPolygonVertex> foundVertices = new array<PRAPolygonVertex>();
			foreach (vector vertPos : polyRec.Vertices) {
				PRAPolygonVertex vertex = FindNearestPolygonVertex(vertPos, 3);
				if (vertex) {
					if (polyRec.AreaGroupName != "") vertex.SetAreaGroup(polyRec.AreaGroupName);
					foundVertices.Insert(vertex);
				}
			}

			// If we found all vertices, polygon is complete
			if (foundVertices.Count() == polyRec.Vertices.Count()) {
				g_PRAPolygon_ToApply.Remove(m);
			}
		}

		// Check if all done
		int totalRemaining = g_PRABox_ToApply.Count() + g_PRASafe3D_ToApply.Count() + g_PRASafe2D_ToApply.Count() + g_PRAPolygon_ToApply.Count();
		if (totalRemaining == 0) {
			return;
		}

		// Retry if attempts remain
		s_Attempts--;
		if (s_Attempts > 0) {
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PRA_PostImportApplier.Tick, 200, false);
		}
	}

	static PRABoxObject FindNearestPRABox(vector pos, float radius)
	{
		ref array<Object> objs = new array<Object>();
		GetGame().GetObjectsAtPosition3D(pos, radius, objs, null);

		float bestD2 = 1000000000000;
		PRABoxObject best;
		foreach (Object o : objs)
		{
			PRABoxObject box = PRABoxObject.Cast(o);
			if (!box) continue;
			float d2 = vector.DistanceSq(box.GetPosition(), pos);
			if (d2 < bestD2) {
				bestD2 = d2;
				best = box;
			}
		}
		return best;
	}

	static PRASafePosition3D FindNearestSafePosition3D(vector pos, float radius)
	{
		ref array<Object> objs = new array<Object>();
		GetGame().GetObjectsAtPosition3D(pos, radius, objs, null);

		float bestD2 = 1000000000000;
		PRASafePosition3D best;
		foreach (Object o : objs)
		{
			PRASafePosition3D safe = PRASafePosition3D.Cast(o);
			if (!safe) continue;
			float d2 = vector.DistanceSq(safe.GetPosition(), pos);
			if (d2 < bestD2) {
				bestD2 = d2;
				best = safe;
			}
		}
		return best;
	}

	static PRASafePosition2D FindNearestSafePosition2D(vector pos, float radius)
	{
		ref array<Object> objs = new array<Object>();
		GetGame().GetObjectsAtPosition3D(pos, radius, objs, null);

		float bestD2 = 1000000000000;
		PRASafePosition2D best;
		foreach (Object o : objs)
		{
			PRASafePosition2D safe = PRASafePosition2D.Cast(o);
			if (!safe) continue;
			float d2 = vector.DistanceSq(safe.GetPosition(), pos);
			if (d2 < bestD2) {
				bestD2 = d2;
				best = safe;
			}
		}
		return best;
	}

	static PRAPolygonVertex FindNearestPolygonVertex(vector pos, float radius)
	{
		ref array<Object> objs = new array<Object>();
		GetGame().GetObjectsAtPosition3D(pos, radius, objs, null);

		float bestD2 = 1000000000000;
		PRAPolygonVertex best;
		foreach (Object o : objs)
		{
			PRAPolygonVertex vertex = PRAPolygonVertex.Cast(o);
			if (!vertex) continue;
			float d2 = vector.DistanceSq(vertex.GetPosition(), pos);
			if (d2 < bestD2) {
				bestD2 = d2;
				best = vertex;
			}
		}
		return best;
	}
}
