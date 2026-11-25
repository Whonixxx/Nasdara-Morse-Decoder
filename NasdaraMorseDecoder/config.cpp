class CfgPatches
{
	class NasdaraMorseDecoder
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Data",
			"DZ_Scripts",
			"DZ_Gear_Nasdara"
		};
	};
};

class CfgMods
{
	class NasdaraMorseDecoder
	{
		dir="NasdaraMorseDecoder";
		picture="";
		action="";
		hideName=1;
		hidePicture=1;
		name="Nasdara Morse Decoder";
		credits="Developed by Whonixxx for Nasdara map fans";
		author="Whonixxx";
		authorID="0";
		version="1.1";
		extra=0;
		type="mod";
		dependencies[]=
		{
			"Game",
			"World",
			"Mission"
		};
		class defs
		{
			class gameScriptModule
			{
				value="";
				files[]=
				{
					"NasdaraMorseDecoder/scripts/3_game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"NasdaraMorseDecoder/scripts/4_world"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"NasdaraMorseDecoder/scripts/5_mission"
				};
			};
		};
	};
};
