class PRAPolygonManager
{
	protected static ref PRAPolygonManager s_Instance;

	// Simple storage: parallel arrays instead of map with ref classes
	protected ref array<string> m_AreaNames;
	protected ref array<ref array<PRAPolygonVertex>> m_VerticesByArea;

	// Map of line connections (key = "startVertex_endVertex")
	protected ref map<string, PRAPolygonLine> m_Lines;

	// Update timer
	protected ref Timer m_UpdateTimer;

	void PRAPolygonManager()
	{
		m_AreaNames = new array<string>();
		m_VerticesByArea = new array<ref array<PRAPolygonVertex>>();
		m_Lines = new map<string, PRAPolygonLine>();

		// Start update timer (5Hz for line updates)
		m_UpdateTimer = new Timer();
		m_UpdateTimer.Run(PRAEditorSettings.GetPolygonUpdateInterval(), this, "UpdateAllLines", null, true);
	}

	void ~PRAPolygonManager()
	{
		if (m_UpdateTimer) {
			m_UpdateTimer.Stop();
		}

		// Cleanup all lines
		CleanupAllLines();
	}

	// Singleton access
	static PRAPolygonManager GetInstance()
	{
		if (!s_Instance) {
			s_Instance = new PRAPolygonManager();
		}
		return s_Instance;
	}

	// Register a vertex
	void RegisterVertex(PRAPolygonVertex vertex)
	{
		if (!vertex) return;

		string areaName = vertex.GetAreaGroup();
		vertex.SetManager(this);

		// Find or create area index
		int areaIndex = m_AreaNames.Find(areaName);

		if (areaIndex == -1) {
			// Create new area
			areaIndex = m_AreaNames.Count();
			m_AreaNames.Insert(areaName);
			m_VerticesByArea.Insert(new array<PRAPolygonVertex>());
			Print(string.Format("[PRA Debug] Created new area: %1 at index %2", areaName, areaIndex));
		}

		// Get vertex array for this area
		array<PRAPolygonVertex> vertices = m_VerticesByArea.Get(areaIndex);

		if (!vertices) {
			Print("[PRA Debug] ERROR: vertices array is null!");
			return;
		}

		// Don't add duplicates
		if (vertices.Find(vertex) == -1) {
			vertices.Insert(vertex);
			Print(string.Format("[PRA Debug] Added vertex to area %1, total count: %2", areaName, vertices.Count()));

			// Update vertex indices
			UpdateVertexIndices(areaName);

			// Rebuild lines for this area
			RebuildLinesForArea(areaName);
		}
	}

	// Unregister a vertex (called when deleted)
	void UnregisterVertex(PRAPolygonVertex vertex)
	{
		if (!vertex) return;

		string areaName = vertex.GetAreaGroup();
		int areaIndex = m_AreaNames.Find(areaName);

		if (areaIndex != -1) {
			array<PRAPolygonVertex> vertices = m_VerticesByArea.Get(areaIndex);
			if (!vertices) return;

			int idx = vertices.Find(vertex);

			if (idx != -1) {
				vertices.Remove(idx);

				// Update vertex indices
				UpdateVertexIndices(areaName);

				// Rebuild lines for this area
				RebuildLinesForArea(areaName);
			}
		}
	}

	// Called when vertex is deleted
	void OnVertexDeleted(PRAPolygonVertex vertex)
	{
		UnregisterVertex(vertex);
	}

	// Called when vertex area changes
	void OnVertexAreaChanged(PRAPolygonVertex vertex)
	{
		// Remove from all areas and re-register
		for (int i = 0; i < m_AreaNames.Count(); i++) {
			array<PRAPolygonVertex> vertices = m_VerticesByArea.Get(i);
			if (!vertices) continue;

			int idx = vertices.Find(vertex);
			if (idx != -1) {
				vertices.Remove(idx);
				string areaName = m_AreaNames.Get(i);
				UpdateVertexIndices(areaName);
				RebuildLinesForArea(areaName);
				break;
			}
		}

		// Re-register with new area
		RegisterVertex(vertex);
	}

	// Called when vertex moves
	void OnVertexMoved(PRAPolygonVertex vertex)
	{
		// Lines will be updated by the timer
	}

	// Update vertex indices within an area
	protected void UpdateVertexIndices(string areaName)
	{
		int areaIndex = m_AreaNames.Find(areaName);
		if (areaIndex == -1) return;

		array<PRAPolygonVertex> vertices = m_VerticesByArea.Get(areaIndex);
		if (!vertices) return;

		for (int i = 0; i < vertices.Count(); i++) {
			vertices[i].SetVertexIndex(i);
		}
	}

	// Rebuild lines for an area
	protected void RebuildLinesForArea(string areaName)
	{
		// Remove old lines for this area
		array<string> linesToRemove = new array<string>();

		foreach (string lineKey, PRAPolygonLine line : m_Lines) {
			if (line && line.GetAreaGroup() == areaName) {
				linesToRemove.Insert(lineKey);
				line.Cleanup();
			}
		}

		foreach (string key : linesToRemove) {
			m_Lines.Remove(key);
		}

		// Create new lines
		int areaIndex = m_AreaNames.Find(areaName);
		if (areaIndex == -1) return;

		array<PRAPolygonVertex> vertices = m_VerticesByArea.Get(areaIndex);
		if (!vertices) return;

		int vertexCount = vertices.Count();

		if (vertexCount < 2) return; // Need at least 2 vertices for a line

		// Create lines between consecutive vertices
		for (int i = 0; i < vertexCount; i++) {
			int nextIdx = (i + 1) % vertexCount; // Wrap around to close polygon

			PRAPolygonVertex startVertex = vertices[i];
			PRAPolygonVertex endVertex = vertices[nextIdx];

			if (startVertex && endVertex) {
				CreateLine(startVertex, endVertex, areaName);
			}
		}
	}

	// Create a line between two vertices
	protected void CreateLine(PRAPolygonVertex startVertex, PRAPolygonVertex endVertex, string areaName)
	{
		// Create line key
		string lineKey = GetLineKey(startVertex, endVertex);

		// Don't create duplicate lines
		if (m_Lines.Contains(lineKey)) return;

		// Spawn line object
		vector spawnPos = startVertex.GetPosition();
		Print(string.Format("[PRA Debug] Creating line at %1 for area: %2", spawnPos.ToString(), areaName));
		PRAPolygonLine line = PRAPolygonLine.Cast(GetGame().CreateObject("PRAPolygonLine", spawnPos));

		if (line) {
			line.SetVertices(startVertex, endVertex);
			m_Lines.Insert(lineKey, line);
			Print(string.Format("[PRA Debug] Successfully created line, total lines: %1", m_Lines.Count()));
		} else {
			Print("[PRA Debug] ERROR: Failed to create PRAPolygonLine object!");
		}
	}

	// Get unique key for a line
	protected string GetLineKey(PRAPolygonVertex v1, PRAPolygonVertex v2)
	{
		// Use memory addresses for unique keys
		string key = v1.ToString() + "_" + v2.ToString();
		return key;
	}

	// Update all line positions
	protected void UpdateAllLines()
	{
		// Clean up invalid lines
		array<string> invalidLines = new array<string>();

		foreach (string lineKey, PRAPolygonLine line : m_Lines) {
			if (!line || !line.IsValid()) {
				invalidLines.Insert(lineKey);
				if (line) line.Cleanup();
			} else {
				line.UpdateLinePosition();
			}
		}

		// Remove invalid lines
		foreach (string key : invalidLines) {
			m_Lines.Remove(key);
		}
	}

	// Cleanup all lines (for shutdown)
	protected void CleanupAllLines()
	{
		foreach (string lineKey, PRAPolygonLine line : m_Lines) {
			if (line) {
				line.Cleanup();
			}
		}

		m_Lines.Clear();
	}

	// Get vertices for an area (for export)
	array<PRAPolygonVertex> GetVerticesForArea(string areaName)
	{
		int areaIndex = m_AreaNames.Find(areaName);
		if (areaIndex != -1) {
			array<PRAPolygonVertex> vertices = m_VerticesByArea.Get(areaIndex);
			if (vertices) {
				return vertices;
			}
		}
		return new array<PRAPolygonVertex>();
	}

	// Get all area names with polygons
	array<string> GetAreaNames()
	{
		array<string> areaNames = new array<string>();

		for (int i = 0; i < m_AreaNames.Count(); i++) {
			array<PRAPolygonVertex> vertices = m_VerticesByArea.Get(i);
			if (vertices && vertices.Count() >= 3) { // Valid polygon needs at least 3 vertices
				areaNames.Insert(m_AreaNames.Get(i));
			}
		}

		return areaNames;
	}
}
