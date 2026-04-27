class PRAPolygonVertexController: Managed
{
	protected PRAPolygonVertex m_Vertex;

	// Properties exposed to UI
	string PRA_AreaGroup = "UnnamedArea";
	int PRA_VertexIndex = 0;

	void PRAPolygonVertexController(PRAPolygonVertex vertex)
	{
		m_Vertex = vertex;

		// Initialize values from the object
		PRA_AreaGroup = vertex.GetAreaGroup();
		PRA_VertexIndex = vertex.GetVertexIndex();
	}

	void PropertyChanged(string property_name)
	{
		if (!m_Vertex) return;

		switch (property_name) {
			case "PRA_AreaGroup": {
				if (PRA_AreaGroup == "") PRA_AreaGroup = "UnnamedArea";
				m_Vertex.SetAreaGroup(PRA_AreaGroup);
				break;
			}
			case "PRA_VertexIndex": {
				// Vertex index is read-only, managed by PRAPolygonManager
				// Reset to current value
				PRA_VertexIndex = m_Vertex.GetVertexIndex();
				break;
			}
		}
	}
}
