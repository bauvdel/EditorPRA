modded class EditorObject
{
    //Remove bounding box for PRA objects
    override void ShowBoundingBox()
    {
        Object w = GetWorldObject();

        // Hide bounding box for PRA boxes
        if (PRABoxObject.Cast(w)) {
            return;
        }

        super.ShowBoundingBox();
    }
}