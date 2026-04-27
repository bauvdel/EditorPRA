// PRAObjectClip - Snapshot data structure for clipboard operations
class PRAObjectClip
{
    string Type;
    vector Position;
    vector Orientation;
    vector Size;
    string Name;
    string AreaGroupName;  // CRITICAL: Preserve area grouping for PRABoxObject
}

class PRAClipboard
{
    protected static ref array<ref PRAObjectClip> s_Buffer = new array<ref PRAObjectClip>();
    protected static vector s_CopyCenter = "0 0 0";

    static void Clear()
    {
        s_Buffer.Clear();
        s_CopyCenter = "0 0 0";
    }

    static int Count()
    {
        return s_Buffer.Count();
    }

    static int Copy(array<EditorObject> selection)
    {
        Clear();
        if (!selection || selection.Count() == 0) {
            return 0;
        }

        s_CopyCenter = ComputeCenter(selection);

        foreach (EditorObject eo : selection) {
            if (!eo) continue;

            Object w = eo.GetWorldObject();
            if (!w) continue;

            PRAObjectClip clip = new PRAObjectClip();
            clip.Type        = eo.GetType();
            clip.Position    = eo.GetPosition();
            clip.Orientation = eo.GetOrientation();
            clip.Name        = eo.GetDisplayName();

            // PRABoxObject
            PRABoxObject praBox = PRABoxObject.Cast(w);
            if (praBox) {
                clip.Size = praBox.GetSize();
                clip.AreaGroupName = praBox.GetAreaGroup();
                s_Buffer.Insert(clip);
                continue;
            }

            // PRASafePosition3D
            PRASafePosition3D safe3D = PRASafePosition3D.Cast(w);
            if (safe3D) {
                clip.AreaGroupName = safe3D.GetAreaGroup();
                s_Buffer.Insert(clip);
                continue;
            }

            // PRASafePosition2D
            PRASafePosition2D safe2D = PRASafePosition2D.Cast(w);
            if (safe2D) {
                clip.AreaGroupName = safe2D.GetAreaGroup();
                s_Buffer.Insert(clip);
                continue;
            }

            // PRAPolygonVertex
            PRAPolygonVertex vertex = PRAPolygonVertex.Cast(w);
            if (vertex) {
                clip.AreaGroupName = vertex.GetAreaGroup();
                s_Buffer.Insert(clip);
                continue;
            }

            s_Buffer.Insert(clip);
        }

        return s_Buffer.Count();
    }

    static int Cut(array<EditorObject> selection, Editor editor)
    {
        int n = Copy(selection);
        if (n == 0) {
            return 0;
        }

        array<EditorObject> objects_to_delete = new array<EditorObject>();
        foreach (EditorObject eo : selection) {
            if (eo) objects_to_delete.Insert(eo);
        }
        if (objects_to_delete.Count() > 0) {
            editor.DeleteObjects(objects_to_delete);
        }
        return n;
    }

    static int Paste(Editor editor, int pasteMode = 1, vector offsetPerIndex = "0 0 0", out array<EditorObject> created_out = null)
    {
        if (s_Buffer.Count() == 0) {
            return 0;
        }
        if (!created_out) {
            created_out = new array<EditorObject>();
        }

        vector anchor;
        if (pasteMode == 1) {
            anchor = GetCursorAnchor(editor);
            vector zero = "0 0 0";
            if (anchor == zero) {
                anchor = s_CopyCenter;
            }
        } else {
            anchor = s_CopyCenter;
        }

        return SpawnFromBuffer(editor, anchor, pasteMode == 1, offsetPerIndex, created_out);
    }

    static int PasteAt(Editor editor, vector anchor, vector offsetPerIndex = "0 0 0", out array<EditorObject> created_out = null)
    {
        if (s_Buffer.Count() == 0) {
            return 0;
        }
        if (!created_out) {
            created_out = new array<EditorObject>();
        }
        return SpawnFromBuffer(editor, anchor, true, offsetPerIndex, created_out);
    }

    protected static vector ComputeCenter(array<EditorObject> objs)
    {
        if (!objs || objs.Count() == 0) {
            vector zeroEarly = "0 0 0";
            return zeroEarly;
        }

        vector sum = "0 0 0";
        int n = 0;

        foreach (EditorObject eo : objs) {
            if (!eo) continue;
            sum = sum + eo.GetPosition();
            n++;
        }

        if (n > 0) {
            float invN = 1 / n;
            vector center;
            center[0] = sum[0] * invN;
            center[1] = sum[1] * invN;
            center[2] = sum[2] * invN;
            return center;
        } else {
            vector zeroLate = "0 0 0";
            return zeroLate;
        }
    }

    protected static vector GetCursorAnchor(Editor editor)
    {
        if (!editor) {
            vector zero = "0 0 0";
            return zero;
        }

        Raycast rc = editor.GetCursorRaycastModeSafe();
        if (rc && rc.Bounce) {
            return rc.Bounce.Position;
        }

        rc = editor.GetCameraRaycast();
        if (rc && rc.Bounce) {
            return rc.Bounce.Position;
        }

        vector zero2 = "0 0 0";
        return zero2;
    }

    protected static vector ScaleVec(vector v, float f)
    {
        vector r;
        r[0] = v[0] * f;
        r[1] = v[1] * f;
        r[2] = v[2] * f;
        return r;
    }

    protected static int SpawnFromBuffer(Editor editor, vector anchor, bool relativeToCenter, vector offsetPerIndex, out array<EditorObject> created_out)
    {
        ref array<ref EditorObjectData> write_data = new array<ref EditorObjectData>();

        int idx = 0;
        foreach (PRAObjectClip clip : s_Buffer) {
            if (!clip) continue;

            vector pos = clip.Position;
            if (relativeToCenter) {
                vector delta = clip.Position - s_CopyCenter;
                float fidx = idx;
                vector step = ScaleVec(offsetPerIndex, fidx);
                pos = anchor + delta + step;
            }

            EditorObjectData data = EditorObjectData.Create(clip.Type, pos, clip.Orientation, 1, EFE_DEFAULT);
            if (data) {
                write_data.Insert(data);
            }
            idx++;
        }

        if (write_data.Count() == 0) {
            return 0;
        }

        EditorObjectMap created_map = editor.CreateObjects(write_data);
        if (!created_map) {
            return 0;
        }

        ref array<EditorObject> created_list = created_map.GetValueArray();
        if (!created_out) {
            created_out = new array<EditorObject>();
        }
        if (created_list) {
            for (int i = 0; i < created_list.Count(); i++) {
                created_out.Insert(created_list[i]);
            }
        }

        int minCount = Math.Min(created_out.Count(), s_Buffer.Count());
        for (int j = 0; j < minCount; j++) {
            EditorObject eo = created_out[j];
            if (!eo) continue;

            PRAObjectClip clipj = s_Buffer[j];
            if (!clipj) continue;

            ApplyCustomProps(eo, clipj);
        }

        editor.ClearSelection();
        foreach (EditorObject eoc : created_out) {
            editor.SelectObject(eoc);
        }

        return created_out.Count();
    }

    protected static void ApplyCustomProps(EditorObject eo, PRAObjectClip clip)
    {
        if (!eo) return;

        Object w = eo.GetWorldObject();
        if (!w) return;

        // PRABoxObject
        PRABoxObject praBox = PRABoxObject.Cast(w);
        if (praBox) {
            vector zeroSize = "0 0 0";
            if (clip.Size != zeroSize) praBox.SetSize(clip.Size);
            if (clip.AreaGroupName != "") praBox.SetAreaGroup(clip.AreaGroupName);
            return;
        }

        // PRASafePosition3D
        PRASafePosition3D safe3D = PRASafePosition3D.Cast(w);
        if (safe3D) {
            if (clip.AreaGroupName != "") safe3D.SetAreaGroup(clip.AreaGroupName);
            return;
        }

        // PRASafePosition2D
        PRASafePosition2D safe2D = PRASafePosition2D.Cast(w);
        if (safe2D) {
            if (clip.AreaGroupName != "") safe2D.SetAreaGroup(clip.AreaGroupName);
            return;
        }

        // PRAPolygonVertex
        PRAPolygonVertex vertex = PRAPolygonVertex.Cast(w);
        if (vertex) {
            if (clip.AreaGroupName != "") vertex.SetAreaGroup(clip.AreaGroupName);
            return;
        }
    }
}
