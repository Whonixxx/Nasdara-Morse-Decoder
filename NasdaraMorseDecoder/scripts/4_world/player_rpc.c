modded class PlayerBase
{
#ifdef EXPANSIONMODNAVIGATION
	// Stores active Expansion personal marker UIDs per bunker index to prevent duplicates
	protected ref map<int, string> m_MorseExpansionMarkerUIDs;
#endif

	// Detects the client's active game language using native UI string translation
	// TODO: Add support for additional community languages (e.g., RU, DE, FR)
	protected string GetMorseClientLanguage()
	{
		string check = Widget.TranslateString("#STR_Controls_Apply");
		check.ToLower();

		if (check.Contains("aplicar"))
		{
			string checkMenu = Widget.TranslateString("#STR_MainMenu_Options");
			checkMenu.ToLower();
			if (checkMenu.Contains("configuraç"))
				return "PT";
			else
				return "ES";
		}

		return "EN";
	}
	// Translates server-sent Portuguese strings into the player's local client language
	protected string TranslateMorseText(string text, out string outTitle)
	{
		string lang = GetMorseClientLanguage();

		if (lang == "PT" || lang == "ES")
			outTitle = "DECODIFICADOR MORSE";
		else
			outTitle = "MORSE DECODER";

		if (lang == "PT")
			return text;

		if (lang == "ES")
		{
			text.Replace("Sinal Morse detectado... Decodificando frequencia...", "Señal Morse detectada... Decodificando frecuencia...");
			text.Replace("Sinal Morse detectado... Decodificando frequência...", "Señal Morse detectada... Decodificando frecuencia...");
			text.Replace("Bunker ", "Búnker ");
			text.Replace("Senha:", "Contraseña:");
			text.Replace("Aguardando...", "Esperando...");
			return text;
		}

		text.Replace("Sinal Morse detectado... Decodificando frequencia...", "Morse signal detected... Decoding frequency...");
		text.Replace("Sinal Morse detectado... Decodificando frequência...", "Morse signal detected... Decoding frequency...");
		text.Replace("Senha:", "Pass:");
		text.Replace("Aguardando...", "Waiting...");
		return text;
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);
		// Displays decoded bunker coordinates and password via on-screen notification and chat
		if (rpc_type == EMorseDecoderRPC.RPC_SHOW_DECODED_BUNKER)
		{
			Param1<string> param;
			if (ctx.Read(param))
			{
				string decodedText = param.param1;
				string title = "DECODIFICADOR MORSE";

				decodedText = TranslateMorseText(decodedText, title);

				NotificationSystem.AddNotificationExtended(7.0, title, decodedText, "");
				GetGame().Chat("[MORSE] " + decodedText, "colorAction");
			}
		}
// Syncs optional map markers (currently supports DayZ Expansion Navigation)
// TODO: Add handlers for LBmaster Advanced Groups and Vanilla++ Map markers here
#ifdef EXPANSIONMODNAVIGATION
		if (rpc_type == EMorseDecoderRPC.RPC_SYNC_BUNKER_MARKER)
		{
			Param4<vector, string, string, int> markerParam;
			if (ctx.Read(markerParam))
			{
				array<string> parts = new array<string>();
				markerParam.param3.Split("|", parts);
				if (parts.Count() >= 3)
				{
					int bunkerIndex = parts.Get(0).ToInt();
					bool show3D = (parts.Get(1).ToInt() == 1);
					string iconName = parts.Get(2);
					CreateOrUpdateMorseExpansionMarker(bunkerIndex, markerParam.param1, markerParam.param2, show3D, iconName, markerParam.param4);
				}
			}
		}
#endif
	}

#ifdef EXPANSIONMODNAVIGATION
// Creates or updates a non-persistent personal marker on the DayZ Expansion map/GPS
	protected void CreateOrUpdateMorseExpansionMarker(int bunkerIndex, vector pos, string lockCode, bool show3D, string iconName, int color)
	{
		ExpansionMarkerModule markerModule = ExpansionMarkerModule.GetModuleInstance();
		if (!markerModule || !markerModule.GetData())
			return;

		if (!m_MorseExpansionMarkerUIDs)
			m_MorseExpansionMarkerUIDs = new map<int, string>();

		if (m_MorseExpansionMarkerUIDs.Contains(bunkerIndex))
		{
			string oldUID = m_MorseExpansionMarkerUIDs.Get(bunkerIndex);
			if (oldUID != "")
			{
				markerModule.RemovePersonalMarkerByUID(oldUID);
			}
		}

		string lang = GetMorseClientLanguage();
		string translatedCode = lockCode;
		string markerLabel;

		if (lang == "ES")
		{
			translatedCode.Replace("Aguardando...", "Esperando...");
			markerLabel = string.Format("Búnker %1 | Contraseña: %2", bunkerIndex, translatedCode);
		}
		else if (lang == "EN")
		{
			translatedCode.Replace("Aguardando...", "Waiting...");
			markerLabel = string.Format("Bunker %1 | Pass: %2", bunkerIndex, translatedCode);
		}
		else
		{
			markerLabel = string.Format("Bunker %1 | Senha: %2", bunkerIndex, translatedCode);
		}

		ExpansionMarkerData markerData = ExpansionMarkerData.Create(ExpansionMapMarkerType.PERSONAL);
		if (markerData)
		{
			markerData.SetName(markerLabel);
			markerData.SetIcon(iconName);
			markerData.SetColor(color);
			markerData.SetPosition(pos);
			markerData.Set3D(show3D);
			markerData.SetLockState(true);
			markerData.m_Persist = false;

			if (markerModule.CreateMarker(markerData))
			{
				m_MorseExpansionMarkerUIDs.Set(bunkerIndex, markerData.GetUID());
			}
		}
	}
#endif
};