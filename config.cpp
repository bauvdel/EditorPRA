class CfgPatches
{
	class EditorPRA
	{
		units[] = {""};
		weapons[] = {};
		requiredVersion = 1.0;
		requiredAddons[] = {"DZ_Scripts","Editor_Scripts","JM_CF_Scripts","DF_Scripts"};
	};
};
class CfgMods
{
	class EditorPRA
	{
		name = "EditorPRA";
		version = "1.0";
		dir = "EditorPRA";
		author = "bauvdel & JinieJ";
		credits = "InclementDab";
		inputs = "EditorPRA/Scripts/Data/Inputs.xml";
		type = "mod";
		dependencies[] = {"Game", "World"};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"EditorPRA/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"EditorPRA/scripts/4_World"};
			};
		};
	};
};
class CfgVehicles
{
	class HouseNoDestruct;
	class PRABoxObject: HouseNoDestruct
	{
		scope = 1;
		displayName = "PRA Restricted Box";
		model = "EditorPRA\models\pra_box.p3d";
	};
	class PRASafePosition3D: HouseNoDestruct
	{
		scope = 1;
		displayName = "PRA Safe Position 3D";
		model = "EditorPRA\models\safe_pos_3d.p3d";
	};
	class PRASafePosition2D: HouseNoDestruct
	{
		scope = 1;
		displayName = "PRA Safe Position 2D";
		model = "EditorPRA\models\safe_pos_2d.p3d";
	};
	class PRAPolygonVertex: HouseNoDestruct
	{
		scope = 1;
		displayName = "PRA Polygon Vertex";
		model = "EditorPRA\models\polygon_vertex.p3d";
	};
	class PRAPolygonLine: HouseNoDestruct
	{
		scope = 1;
		displayName = "PRA Polygon Line";
		model = "EditorPRA\models\polygon_test.p3d";
	};
};