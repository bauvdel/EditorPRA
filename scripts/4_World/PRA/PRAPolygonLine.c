class PRAPolygonLine : Building
{
	protected PRAPolygonVertex m_StartVertex;
	protected PRAPolygonVertex m_EndVertex;
	protected string m_AreaGroupName;

	void PRAPolygonLine()
	{
		SetEventMask(EntityEvent.INIT);
	}

	// Set the vertices this line connects
	void SetVertices(PRAPolygonVertex startVertex, PRAPolygonVertex endVertex)
	{
		m_StartVertex = startVertex;
		m_EndVertex = endVertex;

		if (m_StartVertex) {
			m_AreaGroupName = m_StartVertex.GetAreaGroup();
		}

		UpdateLinePosition();
	}

	PRAPolygonVertex GetStartVertex()
	{
		return m_StartVertex;
	}

	PRAPolygonVertex GetEndVertex()
	{
		return m_EndVertex;
	}

	string GetAreaGroup()
	{
		return m_AreaGroupName;
	}

	// Update line position based on vertex positions
	void UpdateLinePosition()
	{
		if (!m_StartVertex || !m_EndVertex) return;

		vector startPos = m_StartVertex.GetPosition();
		vector endPos = m_EndVertex.GetPosition();

		// Calculate midpoint
		float midX = (startPos[0] + endPos[0]) / 2;
		float midY = (startPos[1] + endPos[1]) / 2;
		float midZ = (startPos[2] + endPos[2]) / 2;
		vector midpoint = Vector(midX, midY, midZ);

		// Set line at midpoint
		SetPosition(midpoint);

		// Calculate rotation to point from start to end
		vector direction = endPos - startPos;
		float distance = direction.Length();

		if (distance > 0.01) {
			// Calculate yaw angle
			float yaw = Math.Atan2(direction[0], direction[2]) * Math.RAD2DEG;

			// Calculate pitch angle (for vertical lines)
			float horizontalDist = Math.Sqrt(direction[0] * direction[0] + direction[2] * direction[2]);
			float pitch = Math.Atan2(direction[1], horizontalDist) * Math.RAD2DEG;

			// Set orientation
			SetOrientation(Vector(yaw, pitch, 0));

			// Scale line to match distance
			// Note: This assumes the base model is 1 meter long along Z-axis
			// We'll scale along Z to match vertex distance
			float scaleZ = distance;
			SetScale(scaleZ);
		}
	}

	// Check if line is still valid (both vertices exist)
	bool IsValid()
	{
		return (m_StartVertex != null && m_EndVertex != null);
	}

	// Cleanup
	void Cleanup()
	{
		m_StartVertex = null;
		m_EndVertex = null;
		GetGame().ObjectDelete(this);
	}
}
