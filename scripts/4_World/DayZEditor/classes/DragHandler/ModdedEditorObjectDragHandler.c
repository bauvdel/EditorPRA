modded class EditorObjectDragHandler
{
    override void OnDragging(notnull EditorObject target, notnull array<EditorObject> additional_drag_targets)
    {
        super.OnDragging(target, additional_drag_targets);

        PRA_FixDuringDrag(target);
        foreach (EditorObject eo : additional_drag_targets) {
            PRA_FixDuringDrag(eo);
        }
    }

    protected vector PRA_RotateAroundAxis(vector v, vector k_unit, float theta)
    {
        float c = Math.Cos(theta);
        float s = Math.Sin(theta);
        vector kxv = Math3D.CrossProduct(k_unit, v);
        float kdv = vector.Dot(k_unit, v);
        float oneMinusC = 1 - c;
        return v * c + kxv * s + k_unit * (kdv * oneMinusC);
    }

    protected void PRA_FixDuringDrag(EditorObject eo)
    {
        if (!eo) return;

        Object w = eo.GetWorldObject();
        PRABoxObject praBox = PRABoxObject.Cast(w);
        if (!praBox) return;

        vector bc0 = eo.GetBottomCenter();
        vector m[4];
        w.GetTransform(m);

        vector ax0 = m[0].Normalized();
        vector ax1 = m[1].Normalized();
        vector ax2 = m[2].Normalized();

        if (m_PRAStartRoll && m_PRAStartRoll.Contains(eo)) {
            float rollNow = PRA_ComputeRoll(ax0, ax1, ax2);
            float dRoll = m_PRAStartRoll[eo] - rollNow;
            float epsilon = 0.000001;
            if (Math.AbsFloat(dRoll) > epsilon) {
                ax0 = PRA_RotateAroundAxis(ax0, ax2, dRoll);
                ax1 = PRA_RotateAroundAxis(ax1, ax2, dRoll);
            }
        }

        vector s = praBox.GetSize();
        m[0] = ax0 * s[0];
        m[1] = ax1 * s[1];
        m[2] = ax2 * s[2];

        w.SetTransform(m);
        w.Update();

        // Don't apply position corrections during CTRL+drag (camera plane placement)
        if (!GetEditor().IsCtrlDown()) {
            vector bc1 = eo.GetBottomCenter();
            vector delta = bc0 - bc1;

            if (GetEditor().IsShiftDown()) {
                if (delta != vector.Zero) {
                    w.GetTransform(m);
                    m[3] = m[3] + delta;
                    w.SetTransform(m);
                    eo.Update();
                }
            } else {
                float dy = delta[1];
                float posEpsilon = 0.0001;
                if (Math.AbsFloat(dy) > posEpsilon) {
                    vector p = eo.GetPosition();
                    p[1] = p[1] + dy;
                    eo.SetPosition(p);
                    eo.Update();
                }
            }
        }
    }
}
