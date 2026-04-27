class PRABoxController: Managed
{
	protected PRABoxObject m_PRABoxObject;

	// Properties exposed to UI
	string PRA_AreaGroup = "UnnamedArea";
	vector PRA_Size;
	vector PRA_Rotation;

	void PRABoxController(PRABoxObject pra_object)
	{
		m_PRABoxObject = pra_object;

		// Initialize values from the object
		PRA_AreaGroup = pra_object.GetAreaGroup();
		PRA_Size = pra_object.GetSize();
		PRA_Rotation = pra_object.GetRotationEuler();
	}

	void PropertyChanged(string property_name)
	{
		if (!m_PRABoxObject) return;

		switch (property_name) {
			case "PRA_AreaGroup": {
				if (PRA_AreaGroup == "") PRA_AreaGroup = "UnnamedArea";
				m_PRABoxObject.SetAreaGroup(PRA_AreaGroup);
				break;
			}

			case "PRA_Size": {
				// Clamp size to minimum 1m per dimension
				if (PRA_Size[0] < 1) PRA_Size[0] = 1;
				if (PRA_Size[1] < 1) PRA_Size[1] = 1;
				if (PRA_Size[2] < 1) PRA_Size[2] = 1;

				m_PRABoxObject.SetSize(PRA_Size);
				break;
			}

			case "PRA_Rotation": {
				m_PRABoxObject.SetRotationEuler(PRA_Rotation);
				break;
			}
		}
	}
}
