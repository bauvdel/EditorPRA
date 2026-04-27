class PRASafePosition2DController: Managed
{
	protected PRASafePosition2D m_SafePosition2D;

	// Properties exposed to UI
	string PRA_AreaGroup = "UnnamedArea";

	void PRASafePosition2DController(PRASafePosition2D safe_pos)
	{
		m_SafePosition2D = safe_pos;

		// Initialize values from the object
		PRA_AreaGroup = safe_pos.GetAreaGroup();
	}

	void PropertyChanged(string property_name)
	{
		if (!m_SafePosition2D) return;

		switch (property_name) {
			case "PRA_AreaGroup": {
				if (PRA_AreaGroup == "") PRA_AreaGroup = "UnnamedArea";
				m_SafePosition2D.SetAreaGroup(PRA_AreaGroup);
				break;
			}
		}
	}
}
