class PRAPropSnapshot
{
    int    Id;
    string Type;
    vector Pos;
    vector Ori;
    vector Size;
    string AreaGroupName;
    float  ExpireAt;
}

class PRAUndoRedo
{
    protected static ref array<ref PRAPropSnapshot> s_Snaps = new array<ref PRAPropSnapshot>();

    static void RememberEO(EditorObject eo, float ttl = 90)
    {
        if (!eo) return;
        Object w = eo.GetWorldObject();
        if (!w) return;

        float now = PRAEditorGameCache.GetCachedTime();

        // PRABoxObject
        PRABoxObject praBox = PRABoxObject.Cast(w);
        if (praBox) {
            PRAPropSnapshot s = new PRAPropSnapshot();
            s.Id = eo.GetID();
            s.Type = praBox.GetType();
            s.Pos = praBox.GetPosition();
            s.Ori = praBox.GetOrientation();
            s.Size = praBox.GetSize();
            s.AreaGroupName = praBox.GetAreaGroup();
            s.ExpireAt = now + ttl;
            s_Snaps.Insert(s);
            return;
        }

        // PRASafePosition3D
        PRASafePosition3D safe3D = PRASafePosition3D.Cast(w);
        if (safe3D) {
            PRAPropSnapshot s3d = new PRAPropSnapshot();
            s3d.Id = eo.GetID();
            s3d.Type = safe3D.GetType();
            s3d.Pos = safe3D.GetPosition();
            s3d.Ori = safe3D.GetOrientation();
            s3d.AreaGroupName = safe3D.GetAreaGroup();
            s3d.ExpireAt = now + ttl;
            s_Snaps.Insert(s3d);
            return;
        }

        // PRASafePosition2D
        PRASafePosition2D safe2D = PRASafePosition2D.Cast(w);
        if (safe2D) {
            PRAPropSnapshot s2d = new PRAPropSnapshot();
            s2d.Id = eo.GetID();
            s2d.Type = safe2D.GetType();
            s2d.Pos = safe2D.GetPosition();
            s2d.Ori = safe2D.GetOrientation();
            s2d.AreaGroupName = safe2D.GetAreaGroup();
            s2d.ExpireAt = now + ttl;
            s_Snaps.Insert(s2d);
            return;
        }

        // PRAPolygonVertex
        PRAPolygonVertex vertex = PRAPolygonVertex.Cast(w);
        if (vertex) {
            PRAPropSnapshot sv = new PRAPropSnapshot();
            sv.Id = eo.GetID();
            sv.Type = vertex.GetType();
            sv.Pos = vertex.GetPosition();
            sv.Ori = vertex.GetOrientation();
            sv.AreaGroupName = vertex.GetAreaGroup();
            sv.ExpireAt = now + ttl;
            s_Snaps.Insert(sv);
            return;
        }
    }

    static void Rehydrate(Editor editor)
    {
        if (!editor || s_Snaps.Count() == 0) return;

        float now = PRAEditorGameCache.GetCachedTime();

        // Purge expired
        for (int i = s_Snaps.Count() - 1; i >= 0; i--) {
            if (s_Snaps[i].ExpireAt <= now) s_Snaps.Remove(i);
        }
        if (s_Snaps.Count() == 0) return;

        // Try to rehydrate by ID
        for (int j = s_Snaps.Count() - 1; j >= 0; j--) {
            PRAPropSnapshot s = s_Snaps[j];

            EditorObject eo = editor.GetPlacedObjectById(s.Id);
            if (!eo) continue;  // not recreated yet

            Object w = eo.GetWorldObject();
            if (!w) {
                s_Snaps.Remove(j);
                continue;
            }

            // PRABoxObject
            PRABoxObject praBox = PRABoxObject.Cast(w);
            if (praBox) {
                vector zero = "0 0 0";
                if (s.Size != zero) praBox.SetSize(s.Size);
                if (s.AreaGroupName != "") praBox.SetAreaGroup(s.AreaGroupName);
                s_Snaps.Remove(j);
                continue;
            }

            // PRASafePosition3D
            PRASafePosition3D safe3D = PRASafePosition3D.Cast(w);
            if (safe3D) {
                if (s.AreaGroupName != "") safe3D.SetAreaGroup(s.AreaGroupName);
                s_Snaps.Remove(j);
                continue;
            }

            // PRASafePosition2D
            PRASafePosition2D safe2D = PRASafePosition2D.Cast(w);
            if (safe2D) {
                if (s.AreaGroupName != "") safe2D.SetAreaGroup(s.AreaGroupName);
                s_Snaps.Remove(j);
                continue;
            }

            // PRAPolygonVertex
            PRAPolygonVertex vertex = PRAPolygonVertex.Cast(w);
            if (vertex) {
                if (s.AreaGroupName != "") vertex.SetAreaGroup(s.AreaGroupName);
                s_Snaps.Remove(j);
                continue;
            }

            // If type changed or not our class, remove it
            if (w.GetType() != s.Type) {
                s_Snaps.Remove(j);
            }
        }
    }
}
