class NasdaraMorseDecoderConfig
{
	static const string CONFIG_DIR = "$profile:NasdaraMorseDecoder";
	static const string CONFIG_PATH = "$profile:NasdaraMorseDecoder/config.json";

	// 1 = Enables markers on DayZ Expansion Navigation (if installed on the server)
	// 0 = Disables map markers (radio and text notifications only)
	int EnableExpansionMarkers = 1;

	// 0 = Shows marker only on 2D map and GPS (recommended to preserve exploration)
	// 1 = Also displays the 3D marker icon projected on the player's screen
	int ExpansionShow3D = 0;

	// Imprecision radius in meters around the bunker (0 = exact coordinates)
	float ApproxRadiusMeters = 150.0;

	// Expansion Navigation marker icon ("Exclamationmark", "Questionmark", "Compass", "Skull")
	string MarkerIcon = "Exclamationmark";

	// RGB marker color on the map (default: orange/amber)
	int MarkerColorR = 255;
	int MarkerColorG = 165;
	int MarkerColorB = 0;

	// TODO: Add configuration options for LBmaster Advanced Groups and Vanilla++ Map markers

	protected static ref NasdaraMorseDecoderConfig m_Instance;

	// Singleton accessor (loads configuration automatically on first call)
	static NasdaraMorseDecoderConfig Get()
	{
		if (!m_Instance)
		{
			m_Instance = new NasdaraMorseDecoderConfig();
			m_Instance.Load();
		}
		return m_Instance;
	}

	// Server-side only: loads config.json from $profile or creates it with default values
	void Load()
	{
		if (!GetGame().IsDedicatedServer() && !GetGame().IsServer())
			return;

		if (!FileExist(CONFIG_DIR))
		{
			MakeDirectory(CONFIG_DIR);
		}

		if (FileExist(CONFIG_PATH))
		{
			JsonFileLoader<NasdaraMorseDecoderConfig>.JsonLoadFile(CONFIG_PATH, this);
		}
		else
		{
			Save();
		}
	}

	void Save()
	{
		if (!FileExist(CONFIG_DIR))
		{
			MakeDirectory(CONFIG_DIR);
		}
		JsonFileLoader<NasdaraMorseDecoderConfig>.JsonSaveFile(CONFIG_PATH, this);
	}

	int GetMarkerARGB()
	{
		return ARGB(255, MarkerColorR, MarkerColorG, MarkerColorB);
	}
};