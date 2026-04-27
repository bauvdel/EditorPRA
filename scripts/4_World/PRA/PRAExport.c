class PRABoxExport
{
	ref array<ref array<float>> Data; // Nested array format

	void PRABoxExport(vector size, vector rotation, vector position)
	{
		// Validate size (minimum 1m per dimension)
		if (size[0] < 1) size[0] = 1;
		if (size[1] < 1) size[1] = 1;
		if (size[2] < 1) size[2] = 1;

		Data = new array<ref array<float>>();

		// [0] = Size [width, height, depth]
		array<float> sizeArr = new array<float>();
		sizeArr.Insert(size[0]);
		sizeArr.Insert(size[1]);
		sizeArr.Insert(size[2]);
		Data.Insert(sizeArr);

		// [1] = Rotation [yaw, pitch, roll]
		array<float> rotArr = new array<float>();
		rotArr.Insert(rotation[0]);
		rotArr.Insert(rotation[1]);
		rotArr.Insert(rotation[2]);
		Data.Insert(rotArr);

		// [2] = Position [X, Y, Z]
		array<float> posArr = new array<float>();
		posArr.Insert(position[0]);
		posArr.Insert(position[1]);
		posArr.Insert(position[2]);
		Data.Insert(posArr);
	}
}

// Single area export
class PRAAreaExport
{
	string areaName;
	ref array<ref array<ref array<float>>> PRABoxes;      // Array of [[size], [rot], [pos]]
	ref array<ref array<ref array<float>>> PRAPolygons;   // Array of [[X, Z], [X, Z], ...]
	ref array<ref array<float>> safePositions3D;          // Array of [X, Y, Z]
	ref array<ref array<float>> safePositions2D;          // Array of [X, Z]

	void PRAAreaExport(string name)
	{
		areaName = name;
		if (areaName == "") areaName = "UnnamedArea";

		PRABoxes = new array<ref array<ref array<float>>>();
		PRAPolygons = new array<ref array<ref array<float>>>();
		safePositions3D = new array<ref array<float>>();
		safePositions2D = new array<ref array<float>>();
	}

	void AddBox(vector size, vector rotation, vector position)
	{
		PRABoxExport box = new PRABoxExport(size, rotation, position);
		PRABoxes.Insert(box.Data);
	}

	void AddPolygon(array<PRAPolygonVertex> vertices)
	{
		// Create array of [X, Z] vertex positions
		array<ref array<float>> polygonVertices = new array<ref array<float>>();

		foreach (PRAPolygonVertex vertex : vertices) {
			if (!vertex) continue;

			vector pos = vertex.GetPosition();
			array<float> vertexPos = new array<float>();
			vertexPos.Insert(pos[0]); // X
			vertexPos.Insert(pos[2]); // Z (skip Y)
			polygonVertices.Insert(vertexPos);
		}

		// Only add if we have at least 3 vertices (valid polygon)
		if (polygonVertices.Count() >= 3) {
			PRAPolygons.Insert(polygonVertices);
		}
	}

	void AddSafePosition3D(vector position)
	{
		array<float> pos = new array<float>();
		pos.Insert(position[0]);
		pos.Insert(position[1]);
		pos.Insert(position[2]);
		safePositions3D.Insert(pos);
	}

	void AddSafePosition2D(vector position)
	{
		array<float> pos = new array<float>();
		pos.Insert(position[0]); // X
		pos.Insert(position[2]); // Z (skip Y)
		safePositions2D.Insert(pos);
	}
}

// Root export structure
class PRAExportRoot
{
	ref array<ref PRAAreaExport> Areas = new array<ref PRAAreaExport>();

	void AddArea(PRAAreaExport area)
	{
		Areas.Insert(area);
	}
}
