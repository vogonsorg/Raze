
//-------------------------------------------------------------------------------------------
//
// Caption drawer
//
//-------------------------------------------------------------------------------------------

class MenuCustomizerBlood : MenuCustomize
{
	override int DrawCaption(String title, Font fnt, int y, bool drawit)
	{
		let font = generic_ui? NewConsoleFont : BigFont;	// this ignores the passed font intentionally.
		let texid = TexMan.CheckForTexture("MENUBAR");
		let texsize = TexMan.GetScaledSize(texid);
		let fonth = font.GetGlyphHeight("A");
		if (drawit)
		{
			int width = font.StringWidth(title);
			if (texid.isValid())
			{
				double scalex = 1.; // Expand the box if the text is longer
				if (texsize.X - 10 < width) scalex = width / (texsize.X - 10);
				screen.DrawTexture(texid, false, 160, 20, DTA_FullscreenScale, FSMode_Fit320x200Top, DTA_CenterOffsetRel, true, DTA_ScaleX, scalex);
			}
			screen.DrawText(font, Font.CR_UNDEFINED, 160 - width / 2, 20 - fonth / 2, title, DTA_FullscreenScale, FSMode_Fit320x200Top);
		}
		double fx, fy, fw, fh;
		[fx, fy, fw, fh] = Screen.GetFullscreenRect(320, 200, FSMode_ScaleToFit43Top);
		int h = texid.isValid()? texsize.Y : fonth;
		return int((y+h) * fh / 200); // This must be the covered height of the header in true pixels.
	}
	
	override bool DrawSelector(ListMenuDescriptor desc)
	{
		return true;	// do not draw any selector.
	}
	
}

//-------------------------------------------------------------------------------------------
//
// The dripping blood - partially native.
//
//-------------------------------------------------------------------------------------------

class ListMenuItemBloodDripDrawer : ListMenuItem
{
	void Init(ListMenuDescriptor desc)
	{
		Super.Init(0, 0);
	}

	native override void Draw(bool selected, ListMenuDescriptor desc);
}



//=============================================================================
//
// text item
//
//=============================================================================

class ListMenuItemBloodTextItem : ListMenuItemTextItem
{
	void Init(ListMenuDescriptor desc, String text, String hotkey, Name child, int param = 0)
	{
		Super.Init(desc, text, hotkey, child, param);
	}
	
	void InitDirect(double x, double y, int height, String hotkey, String text, Font font, int color, int color2, Name child, int param = 0)
	{
		Super.InitDirect(x, y, height, hotkey, text, font, color, color2, child, param);
	}
	
	override void Draw(bool selected, ListMenuDescriptor desc)
	{
		int shade = Selectable()? 32: 48;
		int pal = 5;
		let gamefont = generic_ui ? NewSmallFont : BigFont;
		int xpos = mXpos - gamefont.StringWidth(mText) / 2;
		int cr = generic_ui? Font.CR_GRAY : Font.CR_UNDEFINED;
		int trans = generic_ui? 0 : Translation.MakeID(Translation_Remap, pal);
		
		if (selected) shade = 32 - ((MSTime() * 120 / 1000) & 63);

		Screen.DrawText(gamefont, Font.CR_UNDEFINED, xpos+1, mYpos+1, mText, DTA_Color, 0xff000000, DTA_FullscreenScale, FSMode_Fit320x200);
		Screen.DrawText(gamefont, Font.CR_UNDEFINED, xpos, mYpos, mText, DTA_TranslationIndex, trans, DTA_Color, Build.shadeToLight(shade), DTA_FullscreenScale, FSMode_Fit320x200);
	}
	
}

