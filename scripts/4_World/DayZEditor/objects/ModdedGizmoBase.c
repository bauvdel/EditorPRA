modded class EditorGizmo
{
    // Hide EditorGizmo for PRABoxObjects
    override void Update(float dt)
    {
        auto editor = GetEditor();
        if (!editor)
        {
            super.Update(dt);
            return;
        }

        ref array<EditorObject> ordered = editor.GetSelectedObjectsOrdered();
        if (!ordered || ordered.Count() == 0)
        {
            super.Update(dt);
            return;
        }

        EditorObject top = ordered[ordered.Count() - 1];
        bool isPRABox = top && PRABoxObject.Cast(top.GetWorldObject());

        if (isPRABox)
        {
            m_InteractionIndex = -1;
            m_DragOffset = vector.Zero;
            m_DragRotationOffset = vector.Zero;

            // Hide the gizmo
            if (m_Gizmo)
                m_Gizmo.SetFlags(EntityFlags.VISIBLE, false);

            return;
        }
        else
        {
            if (m_Gizmo)
                m_Gizmo.SetFlags(EntityFlags.VISIBLE, true);

            super.Update(dt);
        }
    }
}
