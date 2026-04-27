modded class EditorObjectPropertiesDialog : EditorDialogBase
{
	protected ref PRABoxController m_PRAController;
	protected ref PRASafePosition3DController m_PRASafe3DController;
	protected ref PRASafePosition2DController m_PRASafe2DController;
	protected ref PRAPolygonVertexController m_PRAPolyVertexController;

	void ~EditorObjectPropertiesDialog()
	{
		// Ensure objects are properly saved when dialog closes
		if (m_EditorObject) {
			m_EditorObject.UpdateNet();
		}
		if (m_EditorObjects) {
			foreach (EditorObject editor_object: m_EditorObjects) {
				editor_object.UpdateNet();
			}
		}
	}

	override void SetEditorObject(EditorObject editor_object)
	{
		Object w = editor_object.GetWorldObject();
		bool isPRABox     = PRABoxObject.Cast(w)    != null;
		bool isSafe3D     = PRASafePosition3D.Cast(w) != null;
		bool isSafe2D     = PRASafePosition2D.Cast(w) != null;
		bool isPolyVertex = PRAPolygonVertex.Cast(w) != null;

		//Return normal GroupPrefab if not one of our PRA objects
		if (!isPRABox && !isSafe3D && !isSafe2D && !isPolyVertex) {
			super.SetEditorObject(editor_object);
			return;
		}
		//Build modded GroupPrefab without scale
		m_EditorObject = editor_object;
		EditorObjectController controller = m_EditorObject.GetController();

		GroupPrefab general_group = new GroupPrefab("#STR_EDITOR_GENERAL", this, string.Empty);
    	general_group.Insert(new CheckBoxPrefab("#STR_EDITOR_SHOW", controller, "Show"));
    	general_group.Insert(new EditBoxPrefab("#STR_EDITOR_NAME", controller, "Name"));
    	general_group.Insert(new VectorPrefab("#STR_EDITOR_POSITION", controller, "Position"));
    	general_group.Insert(new VectorPrefab("#STR_EDITOR_ORIENTATION", controller, "Orientation"));

		AddContent(general_group);

		//Build PRABoxObject GroupPrefab
		if (isPRABox)
		{
			PRABoxObject pra = PRABoxObject.Cast(w);
			m_PRAController = new PRABoxController(pra);

			GroupPrefab pra_group = new GroupPrefab("PRA Restricted Box", this, string.Empty);
			pra_group.Insert(new EditBoxPrefab("Area Group Name", m_PRAController, "PRA_AreaGroup"));
			pra_group.Insert(new VectorPrefab("Size (meters)", m_PRAController, "PRA_Size"));
			pra_group.Insert(new VectorPrefab("Rotation (degrees)", m_PRAController, "PRA_Rotation"));

			AddContent(pra_group);
		}

		//Build PRASafePosition3D GroupPrefab
		if (isSafe3D)
		{
			PRASafePosition3D safe3D = PRASafePosition3D.Cast(w);
			m_PRASafe3DController = new PRASafePosition3DController(safe3D);

			GroupPrefab safe3d_group = new GroupPrefab("PRA Safe Position 3D", this, string.Empty);
			safe3d_group.Insert(new EditBoxPrefab("Area Group Name", m_PRASafe3DController, "PRA_AreaGroup"));

			AddContent(safe3d_group);
		}

		//Build PRASafePosition2D GroupPrefab
		if (isSafe2D)
		{
			PRASafePosition2D safe2D = PRASafePosition2D.Cast(w);
			m_PRASafe2DController = new PRASafePosition2DController(safe2D);

			GroupPrefab safe2d_group = new GroupPrefab("PRA Safe Position 2D", this, string.Empty);
			safe2d_group.Insert(new EditBoxPrefab("Area Group Name", m_PRASafe2DController, "PRA_AreaGroup"));

			AddContent(safe2d_group);
		}

		//Build PRAPolygonVertex GroupPrefab
		if (isPolyVertex)
		{
			PRAPolygonVertex polyVertex = PRAPolygonVertex.Cast(w);
			m_PRAPolyVertexController = new PRAPolygonVertexController(polyVertex);

			GroupPrefab polyvertex_group = new GroupPrefab("PRA Polygon Vertex", this, string.Empty);
			polyvertex_group.Insert(new EditBoxPrefab("Area Group Name", m_PRAPolyVertexController, "PRA_AreaGroup"));
			polyvertex_group.Insert(new EditBoxNumberPrefab("Vertex Index (Read-Only)", m_PRAPolyVertexController, "PRA_VertexIndex", 1, 0, 999));

			AddContent(polyvertex_group);
		}
	}

	override void SetMultipleEditorObjects(array<EditorObject> editor_objects)
	//Same as above, but for multi-select
	{
		int praBoxCount = 0;
		int safe3DCount = 0;
		int safe2DCount = 0;
		int polyVertexCount = 0;

		foreach (EditorObject eo : editor_objects) {
			Object w = eo.GetWorldObject();
			if (PRABoxObject.Cast(w)) praBoxCount++;
			else if (PRASafePosition3D.Cast(w)) safe3DCount++;
			else if (PRASafePosition2D.Cast(w)) safe2DCount++;
			else if (PRAPolygonVertex.Cast(w)) polyVertexCount++;
		}

	// If mixed types or no custom objects, use default behavior
	int totalCount = praBoxCount + safe3DCount + safe2DCount + polyVertexCount;
	int typeCount = 0;
	if (praBoxCount > 0) typeCount++;
	if (safe3DCount > 0) typeCount++;
	if (safe2DCount > 0) typeCount++;
	if (polyVertexCount > 0) typeCount++;

	if (typeCount > 1 || totalCount == 0) {
		super.SetMultipleEditorObjects(editor_objects);
		return;
	}


	m_EditorObjects = editor_objects;
	m_EditorMultiObjectCommandController = new EditorMultiObjectCommandController(editor_objects);

	GroupPrefab general_group = new GroupPrefab("#STR_EDITOR_GENERAL", m_EditorMultiObjectCommandController, string.Empty);
    general_group.Insert(new CheckBoxPrefab("#STR_EDITOR_SHOW", m_EditorMultiObjectCommandController, "Show"));
    general_group.Insert(new EditBoxPrefab("#STR_EDITOR_NAME", m_EditorMultiObjectCommandController, "Name"));
    general_group.Insert(new VectorPrefab("#STR_EDITOR_POSITION", m_EditorMultiObjectCommandController, "Position"));
    general_group.Insert(new VectorPrefab("#STR_EDITOR_ORIENTATION", m_EditorMultiObjectCommandController, "Orientation"));

    AddContent(general_group);

		// ----- All PRA boxes -----
		if (praBoxCount > 0)
		{
			// Seed from first PRA box
			PRABoxObject pra0;
			foreach (EditorObject eo_pra : editor_objects) {
				pra0 = PRABoxObject.Cast(eo_pra.GetWorldObject());
				if (pra0) break;
			}
			if (!pra0) return;

			m_PRAController = new PRABoxController(pra0);

			GroupPrefab pra_group_multi = new GroupPrefab("PRA Restricted Box (Selection)", this, string.Empty);
			pra_group_multi.Insert(new EditBoxPrefab("Area Group Name", m_PRAController, "PRA_AreaGroup"));
			pra_group_multi.Insert(new VectorPrefab("Size (meters)", m_PRAController, "PRA_Size"));
			pra_group_multi.Insert(new VectorPrefab("Rotation (degrees)", m_PRAController, "PRA_Rotation"));
			AddContent(pra_group_multi);
			return;
		}

		// ----- All safe positions 3D -----
		if (safe3DCount > 0)
		{
			// Seed from first safe position 3D
			PRASafePosition3D safe3D0;
			foreach (EditorObject eo_s3d : editor_objects) {
				safe3D0 = PRASafePosition3D.Cast(eo_s3d.GetWorldObject());
				if (safe3D0) break;
			}
			if (!safe3D0) return;

			m_PRASafe3DController = new PRASafePosition3DController(safe3D0);

			GroupPrefab safe3d_group_multi = new GroupPrefab("PRA Safe Position 3D (Selection)", this, string.Empty);
			safe3d_group_multi.Insert(new EditBoxPrefab("Area Group Name", m_PRASafe3DController, "PRA_AreaGroup"));
			AddContent(safe3d_group_multi);
			return;
		}

		// ----- All safe positions 2D -----
		if (safe2DCount > 0)
		{
			// Seed from first safe position 2D
			PRASafePosition2D safe2D0;
			foreach (EditorObject eo_s2d : editor_objects) {
				safe2D0 = PRASafePosition2D.Cast(eo_s2d.GetWorldObject());
				if (safe2D0) break;
			}
			if (!safe2D0) return;

			m_PRASafe2DController = new PRASafePosition2DController(safe2D0);

			GroupPrefab safe2d_group_multi = new GroupPrefab("PRA Safe Position 2D (Selection)", this, string.Empty);
			safe2d_group_multi.Insert(new EditBoxPrefab("Area Group Name", m_PRASafe2DController, "PRA_AreaGroup"));
			AddContent(safe2d_group_multi);
			return;
		}

		// ----- All polygon vertices -----
		if (polyVertexCount > 0)
		{
			// Seed from first polygon vertex
			PRAPolygonVertex polyVertex0;
			foreach (EditorObject eo_pv : editor_objects) {
				polyVertex0 = PRAPolygonVertex.Cast(eo_pv.GetWorldObject());
				if (polyVertex0) break;
			}
			if (!polyVertex0) return;

			m_PRAPolyVertexController = new PRAPolygonVertexController(polyVertex0);

			GroupPrefab polyvertex_group_multi = new GroupPrefab("PRA Polygon Vertex (Selection)", this, string.Empty);
			polyvertex_group_multi.Insert(new EditBoxPrefab("Area Group Name", m_PRAPolyVertexController, "PRA_AreaGroup"));
			AddContent(polyvertex_group_multi);
			return;
		}
	}
}