modded class MissionServer
{
	// Optional extra delay (in seconds) after a bunker's Morse sequence finishes before revealing its code
	protected const float MORSE_END_OFFSET_SEC = 0.0;

	protected ref map<int, vector> m_BunkerApproxPosById = new map<int, vector>();
	protected ref array<string> m_ListeningPlayerUIDs = new array<string>();

	protected int m_LastScheduledReportHour = -1;
	protected int m_LastScheduledReportMinute = -1;
	protected string m_LastScheduledSignature = "";
	protected int m_BroadcastGeneration = 0;
	protected bool m_IsBroadcastPlaying = false;

	protected float m_CheckTimer = 0;
	protected const float CHECK_INTERVAL = 1.0;

	override void OnInit()
	{
		super.OnInit();
		NasdaraMorseDecoderConfig.Get();
		Print("[NasdaraMorseDecoder] v1.1 initialized.");
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		m_CheckTimer += timeslice;
		if (m_CheckTimer >= CHECK_INTERVAL)
		{
			m_CheckTimer = 0;
			MonitorBroadcastAndListeners();
		}
	}

	// Monitors active bunker broadcasts from the Nasdara map handler and notifies listening players
	protected void MonitorBroadcastAndListeners()
	{
		if (!CfgBunkerBroadcastHandler.IsInitialized())
			return;

		BunkerBroadcastHandler handler = BunkerBroadcastHandler.GetInstance();
		if (!handler)
			return;

		BunkerComponentsArray activeBunkers = handler.GetActiveBunkers();
		if (!activeBunkers || activeBunkers.Count() == 0)
		{
			if (m_IsBroadcastPlaying || m_ListeningPlayerUIDs.Count() > 0)
			{
				m_IsBroadcastPlaying = false;
				m_LastScheduledSignature = "";
				m_ListeningPlayerUIDs.Clear();
			}
			return;
		}

		string currentSig = "";
		foreach (BunkerBroadcastComponentBase b : activeBunkers)
		{
			if (b)
				currentSig += b.GetId().ToString() + ":" + b.GetLockCode() + ";";
		}

		BunkerBroadcastPersistenceStorage storage = BunkerBroadcastPersistenceStorage.GetInstance();
		if (storage && storage.m_SchedulerData)
		{
			int repHour = storage.m_SchedulerData.m_LastReportHour;
			int repMin = storage.m_SchedulerData.m_LastReportMinute;

			if (repHour != -1 && (repHour != m_LastScheduledReportHour || repMin != m_LastScheduledReportMinute || currentSig != m_LastScheduledSignature))
			{
				m_LastScheduledReportHour = repHour;
				m_LastScheduledReportMinute = repMin;
				m_LastScheduledSignature = currentSig;
				ScheduleSequentialBunkerReveals(activeBunkers);
			}
		}

		CheckPlayersListening();
	}

	// Calculates exact Morse audio duration for each active bunker and schedules sequential notifications
	protected void ScheduleSequentialBunkerReveals(BunkerComponentsArray activeBunkers)
	{
		BunkerBroadcastConfigInstance cfg = CfgBunkerBroadcastHandler.GetData();
		if (!cfg || !cfg.playbackSettings || !activeBunkers || activeBunkers.Count() == 0)
			return;

		m_BroadcastGeneration++;
		int currentGen = m_BroadcastGeneration;
		m_IsBroadcastPlaying = true;

		float dotSec = cfg.playbackSettings.dotDuration;
		float dashSec = cfg.playbackSettings.dashDuration;
		float symbolGapSec = cfg.playbackSettings.symbolGapDuration;
		float wordGapSec = cfg.playbackSettings.wordGapDuration;
		float bunkerPause = cfg.playbackSettings.pauseBetweenBunkers;
		int bunkerReps = cfg.playbackSettings.bunkerRepetitions;
		int messageReps = cfg.playbackSettings.messageRepetitions;

		if (bunkerReps < 1)
			bunkerReps = 1;
		if (messageReps < 1)
			messageReps = 1;

		int bunkerCount = activeBunkers.Count();
		float elapsedSec = 0.0;

		for (int msgRep = 0; msgRep < messageReps; msgRep++)
		{
			for (int i = 0; i < bunkerCount; i++)
			{
				BunkerBroadcastComponentBase bunker = activeBunkers.Get(i);
				if (!bunker)
					continue;

				string rawCoords = GetRawSixDigitCoords(bunker.GetPosition());
				string lockCode = bunker.GetLockCode();

				string morseCoords = TextToMorse(rawCoords);
				string morseCode = TextToMorse(lockCode);

				float coordsSec = CalcMorseSequenceSeconds(morseCoords, dotSec, dashSec, symbolGapSec);
				float codeSec = CalcMorseSequenceSeconds(morseCode, dotSec, dashSec, symbolGapSec);
				float singleBunkerPassSec = coordsSec + wordGapSec + codeSec;

				float bunkerTotalSec = (singleBunkerPassSec * bunkerReps) + (wordGapSec * (bunkerReps - 1));
				elapsedSec += bunkerTotalSec;

				float adjustedEndSec = Math.Max(0.0, elapsedSec + MORSE_END_OFFSET_SEC);
				int endDelayMs = (int)Math.Round(adjustedEndSec * 1000.0);

				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OnBunkerMorseSequenceFinished, endDelayMs, false, bunker, i + 1, currentGen);

				elapsedSec += bunkerPause;
			}
		}

		int totalCycleMs = (int)Math.Round(elapsedSec * 1000.0);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OnBroadcastCycleFinished, totalCycleMs, false, currentGen);
	}

	void OnBroadcastCycleFinished(int gen)
	{
		if (gen != m_BroadcastGeneration)
			return;

		m_IsBroadcastPlaying = false;
		m_ListeningPlayerUIDs.Clear();
	}

	// Triggered when a bunker's Morse audio finishes; sends decoded coordinates/code and optional map marker
	void OnBunkerMorseSequenceFinished(BunkerBroadcastComponentBase bunker, int bunkerNumber, int gen)
	{
		if (gen != m_BroadcastGeneration)
			return;

		if (!bunker || !bunker.IsActive())
			return;

		NasdaraMorseDecoderConfig cfg = NasdaraMorseDecoderConfig.Get();
		vector exactPos = bunker.GetPosition();

		TIntArray mapCoords = MapNavigationBehaviour.OrderedPositionNumbersFromGridCoords(exactPos);
		string coordsStr = "???";
		if (mapCoords && mapCoords.Count() >= 6)
		{
			coordsStr = string.Format("%1%2%3 / %4%5%6", mapCoords.Get(0), mapCoords.Get(1), mapCoords.Get(2), mapCoords.Get(3), mapCoords.Get(4), mapCoords.Get(5));
		}

		string lockCode = bunker.GetLockCode();
		if (lockCode == "" || lockCode == "-")
			lockCode = "Aguardando...";

		string msg = string.Format("Bunker %1 -> Coord: %2 | Senha: %3", bunkerNumber, coordsStr, lockCode);
		vector approxPos = GetOrCreateStableApproxPos(bunker.GetId(), exactPos, cfg.ApproxRadiusMeters);

		array<Man> players = new array<Man>();
		g_Game.GetWorld().GetPlayerList(players);

		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.IsAlive() || !player.GetIdentity())
				continue;

			if (CanPlayerDecodeMorse(player))
			{
				SendSingleBunkerToPlayer(player, msg);

				// TODO: Add support for LBmaster Advanced Groups and Vanilla++ Map markers here
				if (cfg && cfg.EnableExpansionMarkers == 1)
				{
					SendBunkerMarkerToPlayer(player, bunkerNumber, approxPos, lockCode);
				}
			}
		}
	}

	// Alerts players when they first tune in with an active Handheld Transceiver during a broadcast
	protected void CheckPlayersListening()
	{
		if (!m_IsBroadcastPlaying)
			return;

		array<Man> players = new array<Man>();
		g_Game.GetWorld().GetPlayerList(players);

		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.IsAlive() || !player.GetIdentity())
				continue;

			string uid = player.GetIdentity().GetId();

			if (CanPlayerDecodeMorse(player))
			{
				if (m_ListeningPlayerUIDs.Find(uid) == -1)
				{
					m_ListeningPlayerUIDs.Insert(uid);
					SendSingleBunkerToPlayer(player, "Sinal Morse detectado... Decodificando frequencia...");
				}
			}
			else
			{
				int idx = m_ListeningPlayerUIDs.Find(uid);
				if (idx != -1)
					m_ListeningPlayerUIDs.Remove(idx);
			}
		}
	}

	protected string GetRawSixDigitCoords(vector pos)
	{
		TIntArray mapCoords = MapNavigationBehaviour.OrderedPositionNumbersFromGridCoords(pos);
		if (mapCoords && mapCoords.Count() >= 6)
		{
			return string.Format("%1%2%3%4%5%6", mapCoords.Get(0), mapCoords.Get(1), mapCoords.Get(2), mapCoords.Get(3), mapCoords.Get(4), mapCoords.Get(5));
		}
		return "000000";
	}

	protected string TextToMorse(string text)
	{
		text.ToLower();
		string result = "";
		int len = text.Length();

		for (int i = 0; i < len; i++)
		{
			string symbol = CharToMorse(text.Get(i));
			if (symbol != "")
			{
				if (result != "")
					result += " ";
				result += symbol;
			}
		}
		return result;
	}

	protected string CharToMorse(string ch)
	{
		switch (ch)
		{
		case "0":
			return "-----";
		case "1":
			return ".----";
		case "2":
			return "..---";
		case "3":
			return "...--";
		case "4":
			return "....-";
		case "5":
			return ".....";
		case "6":
			return "-....";
		case "7":
			return "--...";
		case "8":
			return "---..";
		case "9":
			return "----.";
		case "a":
			return ".-";
		case "b":
			return "-...";
		case "c":
			return "-.-.";
		case "d":
			return "-..";
		case "e":
			return ".";
		case "f":
			return "..-.";
		case "g":
			return "--.";
		case "h":
			return "....";
		case "i":
			return "..";
		case "j":
			return ".---";
		case "k":
			return "-.-";
		case "l":
			return ".-..";
		case "m":
			return "--";
		case "n":
			return "-.";
		case "o":
			return "---";
		case "p":
			return ".--.";
		case "q":
			return "--.-";
		case "r":
			return ".-.";
		case "s":
			return "...";
		case "t":
			return "-";
		case "u":
			return "..-";
		case "v":
			return "...-";
		case "w":
			return ".--";
		case "x":
			return "-..-";
		case "y":
			return "-.--";
		case "z":
			return "--..";
		}
		return "";
	}

	protected float CalcMorseSequenceSeconds(string morseStr, float dotSec, float dashSec, float symbolGapSec)
	{
		float total = 0.0;
		int len = morseStr.Length();

		for (int i = 0; i < len; i++)
		{
			string ch = morseStr.Get(i);
			if (ch == ".")
			{
				total += dotSec;
			}
			else if (ch == "-")
			{
				total += dashSec;
			}
			else if (ch == " ")
			{
				total += symbolGapSec;
			}
		}

		return total;
	}

	// Generates and caches a randomized offset position within ApproxRadiusMeters for each bunker ID
	protected vector GetOrCreateStableApproxPos(int bunkerId, vector exactPos, float radiusMeters)
	{
		if (radiusMeters <= 0)
			return exactPos;

		if (m_BunkerApproxPosById.Contains(bunkerId))
			return m_BunkerApproxPosById.Get(bunkerId);

		float angle = Math.RandomFloatInclusive(0, Math.PI2);
		float dist = Math.Sqrt(Math.RandomFloat01()) * radiusMeters;
		vector approxPos = exactPos;
		approxPos[0] = exactPos[0] + (Math.Cos(angle) * dist);
		approxPos[2] = exactPos[2] + (Math.Sin(angle) * dist);
		approxPos[1] = GetGame().SurfaceY(approxPos[0], approxPos[2]);
		m_BunkerApproxPosById.Set(bunkerId, approxPos);
		return approxPos;
	}
	protected void SendSingleBunkerToPlayer(PlayerBase player, string msg)
	{
		if (!player || !player.IsAlive() || !player.GetIdentity() || !CanPlayerDecodeMorse(player))
			return;
		Param1<string> param = new Param1<string>(msg);
		GetGame().RPCSingleParam(player, EMorseDecoderRPC.RPC_SHOW_DECODED_BUNKER, param, true, player.GetIdentity());
	}
	protected void SendBunkerMarkerToPlayer(PlayerBase player, int bunkerIndex, vector approxPos, string lockCode)
	{
		if (!player || !player.IsAlive() || !player.GetIdentity() || !CanPlayerDecodeMorse(player))
			return;
		NasdaraMorseDecoderConfig cfg = NasdaraMorseDecoderConfig.Get();
		if (!cfg || cfg.EnableExpansionMarkers != 1)
			return;
		string meta = string.Format("%1|%2|%3", bunkerIndex, cfg.ExpansionShow3D, cfg.MarkerIcon);
		Param4<vector, string, string, int> markerParam = new Param4<vector, string, string, int>(approxPos, lockCode, meta, cfg.GetMarkerARGB());
		GetGame().RPCSingleParam(player, EMorseDecoderRPC.RPC_SYNC_BUNKER_MARKER, markerParam, true, player.GetIdentity());
	}
	// Checks if the player is holding a powered-on Handheld Transceiver
	// TODO: Validate tuned radio frequency against the bunker broadcast frequency if needed
	protected bool CanPlayerDecodeMorse(PlayerBase player)
	{
		if (!player)
			return false;
		EntityAI itemInHands = player.GetItemInHands();
		if (!itemInHands)
			return false;
		PersonalRadio radio = PersonalRadio.Cast(itemInHands);
		if (radio && radio.HasEnergyManager() && radio.GetCompEM().IsWorking())
			return true;
		return false;
	}
};