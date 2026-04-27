modded class Editor
{
    override void ProcessInput(float dt, Input input)
    {
        super.ProcessInput(dt, input);

        UAInputAPI input_api = GetUApi();
        if (!input_api) return;

        // - Select All
        if (input_api.GetInputByName("PRA_SelectAll").LocalPress()) {
            PRA_SelectAllPRAObjects();
        }

        // - Copy
        if (input_api.GetInputByName("PRA_Copy").LocalPress()) {
            EditorObjectMap selected = GetSelectedObjects();
            if (selected && selected.Count() > 0) {
                array<EditorObject> selection_array = selected.GetValueArray();
                int copied = PRAClipboard.Copy(selection_array);
                GetEditorHud().CreateNotification(string.Format("[PRA] Copied %1 entities", copied));
            } else {
                GetEditorHud().CreateNotification("[PRA] Nothing selected to Copy");
            }
        }

        // - Paste
        if (input_api.GetInputByName("PRA_Paste").LocalPress()) {
            ref array<EditorObject> created = new array<EditorObject>();
            int pasted = PRAClipboard.Paste(this, 1, "0 0 0", created);

            if (pasted <= 0) {
                GetEditorHud().CreateNotification("[PRA] Nothing to Paste");
            } else if (pasted == 1) {
                GetEditorHud().CreateNotification("[PRA] Pasted 1 Entity");
            } else {
                GetEditorHud().CreateNotification(string.Format("[PRA] Pasted %1 entities", pasted));
            }
        }

        // - Cut
        if (input_api.GetInputByName("PRA_Cut").LocalPress()) {
            EditorObjectMap selC = GetSelectedObjects();
            if (selC && selC.Count() > 0) {
                array<EditorObject> arrC = selC.GetValueArray();
                int cut = PRAClipboard.Cut(arrC, this);
                ClearSelection();
                GetEditorHud().CreateNotification(string.Format("[PRA] Cut %1 Entities", cut));
            } else {
                GetEditorHud().CreateNotification("[PRA] Nothing selected to Cut");
            }
        }

        // - Duplicate
        if (input_api.GetInputByName("PRA_Duplicate").LocalPress()) {
            EditorObjectMap selD = GetSelectedObjects();
            if (selD && selD.Count() > 0) {
                array<EditorObject> arrD = selD.GetValueArray();

                PRAClipboard.Copy(arrD);

                ref array<EditorObject> dup_out = new array<EditorObject>();
                int dup = PRAClipboard.Paste(this, 0, "0 0 0", dup_out);

                if (dup > 0) {
                    ClearSelection();
                    foreach (EditorObject eoc : dup_out) {
                        SelectObject(eoc);
                    }
                    GetEditorHud().CreateNotification(string.Format("[PRA] Duplicated %1 Entities", dup));
                } else {
                    GetEditorHud().CreateNotification("[PRA] Duplicate failed");
                }
            } else {
                GetEditorHud().CreateNotification("[PRA] Nothing selected to Duplicate");
            }
        }

        // - Scaling the Size of the PRABoxObject - X Y Z
        EditorObjectMap selectedNow = GetSelectedObjects();
        if (selectedNow && selectedNow.Count() > 0)
        {
            foreach (int sid, EditorObject seo : selectedNow)
            {
                if (!seo) continue;

                Object sw = seo.GetWorldObject();
                PRABoxObject praBox = PRABoxObject.Cast(sw);
                if (!praBox) continue;

                float stepSize = PRAEditorSettings.GetStepSize();
                if (input_api.GetInputByName("PRA_IncLength").LocalPress())  praBox.TrigSize(stepSize, 0, 0);
                if (input_api.GetInputByName("PRA_DecLength").LocalPress())  praBox.TrigSize(-stepSize, 0, 0);
                if (input_api.GetInputByName("PRA_IncWidth").LocalPress())   praBox.TrigSize(0, 0, stepSize);
                if (input_api.GetInputByName("PRA_DecWidth").LocalPress())   praBox.TrigSize(0, 0, -stepSize);
                if (input_api.GetInputByName("PRA_IncHeight").LocalPress())  praBox.TrigSize(0, stepSize, 0);
                if (input_api.GetInputByName("PRA_DecHeight").LocalPress())  praBox.TrigSize(0, -stepSize, 0);
            }
        }

        PRA_ReapplyScaleForSelection();
        PRAUndoRedo.Rehydrate(this);
    }

    // - Keep things scaled after moving
    void PRA_ReapplyScaleForSelection()
    {
        EditorObjectMap selected = GetSelectedObjects();
        if (!selected || selected.Count() == 0) return;

        foreach (int id, EditorObject eo : selected)
        {
            Object w = eo.GetWorldObject();
            PRABoxObject praBox = PRABoxObject.Cast(w);
            if (!praBox) continue;

            vector m[4];
            w.GetTransform(m);

            vector ax0 = m[0].Normalized();
            vector ax1 = m[1].Normalized();
            vector ax2 = m[2].Normalized();

            vector s = praBox.GetSize();
            m[0] = ax0 * s[0];
            m[1] = ax1 * s[1];
            m[2] = ax2 * s[2];

            w.SetTransform(m);
            w.Update();
        }
    }

    override int DeleteObjects(notnull array<EditorObject> editor_objects, bool create_undo = true, bool send_net_message = true)
    {
        foreach (EditorObject eo : editor_objects) {
            if (!eo) continue;
            if (eo.IsLocked()) continue;
            PRAUndoRedo.RememberEO(eo);
        }
        return super.DeleteObjects(editor_objects, create_undo, send_net_message);
    }

    override int DeleteObjects(EditorObjectMap editor_object_map, bool create_undo = true, bool send_net_message = true)
    {
        foreach (int id, EditorObject eo : editor_object_map) {
            if (!eo) continue;
            if (!eo.IsVisible() || eo.IsLocked()) continue;
            PRAUndoRedo.RememberEO(eo);
        }
        return super.DeleteObjects(editor_object_map, create_undo, send_net_message);
    }

    void PRA_SelectAllPRAObjects()
    {
        EditorObjectMap placed = GetPlacedObjects();
        if (!placed || placed.Count() == 0) {
            GetEditorHud().CreateNotification("[PRA] No placed objects found");
            return;
        }

        ClearSelection();

        int picked = 0;
        foreach (EditorObject eo: placed) {
            if (!eo) continue;

            Object w = eo.GetWorldObject();
            if (!w) continue;

            if (w.IsInherited(PRABoxObject) || w.IsInherited(PRASafePosition3D) || w.IsInherited(PRASafePosition2D) || w.IsInherited(PRAPolygonVertex)) {
                SelectObject(eo);
                picked++;
            }
        }

        if (picked > 0) {
            GetEditorHud().CreateNotification(string.Format("[PRA] Selected %1 PRA objects", picked));
        } else {
            GetEditorHud().CreateNotification("[PRA] No PRA objects to select");
        }
    }

    // Sync all PRA objects to use the same AreaGroupName
    void PRA_SyncAllAreaNames()
    {
        EditorObjectMap placed = GetPlacedObjects();
        if (!placed || placed.Count() == 0) return;

        // Find the first area name from boxes or polygons
        string masterAreaName = "";

        // Priority 1: Get from first PRABoxObject
        foreach (int id, EditorObject eo : placed) {
            if (!eo) continue;
            Object w = eo.GetWorldObject();
            if (!w) continue;

            PRABoxObject box = PRABoxObject.Cast(w);
            if (box) {
                masterAreaName = box.GetAreaGroup();
                if (masterAreaName != "" && masterAreaName != "UnnamedArea") {
                    break;
                }
            }
        }

        // Priority 2: If no box found, get from first PRAPolygonVertex
        if (masterAreaName == "" || masterAreaName == "UnnamedArea") {
            PRAPolygonManager polyManager = PRAPolygonManager.GetInstance();
            if (polyManager) {
                array<string> areaNames = polyManager.GetAreaNames();
                if (areaNames && areaNames.Count() > 0) {
                    masterAreaName = areaNames[0];
                }
            }
        }

        // If still no area name, use UnnamedArea
        if (masterAreaName == "") {
            masterAreaName = "UnnamedArea";
        }

        // Apply to all PRA objects (using direct setter to avoid recursion)
        int synced = 0;
        foreach (int id2, EditorObject eo2 : placed) {
            if (!eo2) continue;
            Object w2 = eo2.GetWorldObject();
            if (!w2) continue;

            PRABoxObject box2 = PRABoxObject.Cast(w2);
            if (box2) {
                if (box2.GetAreaGroup() != masterAreaName) {
                    box2.SetAreaGroupDirect(masterAreaName); // Direct set to avoid triggering sync
                    synced++;
                }
                continue;
            }

            PRASafePosition3D safe3D = PRASafePosition3D.Cast(w2);
            if (safe3D) {
                if (safe3D.GetAreaGroup() != masterAreaName) {
                    safe3D.SetAreaGroupDirect(masterAreaName);
                    synced++;
                }
                continue;
            }

            PRASafePosition2D safe2D = PRASafePosition2D.Cast(w2);
            if (safe2D) {
                if (safe2D.GetAreaGroup() != masterAreaName) {
                    safe2D.SetAreaGroupDirect(masterAreaName);
                    synced++;
                }
                continue;
            }

            PRAPolygonVertex vertex = PRAPolygonVertex.Cast(w2);
            if (vertex) {
                if (vertex.GetAreaGroup() != masterAreaName) {
                    vertex.SetAreaGroupDirect(masterAreaName);
                    synced++;
                }
                continue;
            }
        }

        // Only show notification if we actually synced objects (should be rare with auto-sync)
        if (synced > 0) {
            GetEditorHud().CreateNotification(string.Format("[PRA] Export sync: %1 objects updated to area: %2", synced, masterAreaName));
        }
    }
}
