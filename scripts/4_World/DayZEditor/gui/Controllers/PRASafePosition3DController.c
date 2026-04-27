class PRASafePosition3DController: Managed
{
	protected PRASafePosition3D m_SafePosition3D;

	// Properties exposed to UI
	string PRA_AreaGroup = "UnnamedArea";

	void PRASafePosition3DController(PRASafePosition3D safe_pos)
	{
		m_SafePosition3D = safe_pos;

		// Initialize values from the object
		PRA_AreaGroup = safe_pos.GetAreaGroup();
	}

	void PropertyChanged(string property_name)
	{
		if (!m_SafePosition3D) return;

		switch (property_name) {
			case "PRA_AreaGroup": {
				if (PRA_AreaGroup == "") PRA_AreaGroup = "UnnamedArea";
				m_SafePosition3D.SetAreaGroup(PRA_AreaGroup);
				break;
			}
		}
	}
}
