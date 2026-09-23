#include "MainView.h"

using namespace juce;

namespace mm
{
// ======================================================================== PRESET BOX
void PresetBox::paint (Graphics& g)
{
    const auto& t = themeOf (*this);
    auto r = getLocalBounds().toFloat();
    g.setColour (t.display);
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (hover ? t.accent : t.edge.brighter (0.2f));
    g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);

    auto area = getLocalBounds().reduced (9, 0);
    const auto cat = manager.getCurrentCategory();
    g.setFont (FontOptions (10.0f));
    g.setColour (t.accentSoft);
    if (cat.isNotEmpty())
        g.drawText (cat, area.removeFromRight (70), Justification::centredRight);
    g.setFont (FontOptions (12.0f, Font::bold));
    g.setColour (t.text);
    g.drawText (manager.getCurrentName(), area, Justification::centredLeft);
}

// ======================================================================== HEADER
HeaderBar::HeaderBar (MetalMachineAudioProcessor& p) : processor (p), presetBox (p.getPresetManager())
{
    presetBox.onClick = [this] { if (onPresetClicked) onPresetClicked(); };
    prevButton.setTooltip ("Preset anterior");
    nextButton.setTooltip ("Proximo preset");
    prevButton.onClick = [this] { processor.getPresetManager().step (-1); };
    nextButton.onClick = [this] { processor.getPresetManager().step (+1); };

    initButton.setTooltip ("Volta todos os parametros ao patch inicial");
    initButton.onClick = [this]
    {
        processor.resetToInitPatch();
        processor.getPresetManager().markInit();
    };
    saveButton.setTooltip ("Salva por cima do preset do usuario atual (ou pede um nome)");
    saveButton.onClick = [this] { save(); };
    saveAsButton.setTooltip ("Salva como novo preset do usuario");
    saveAsButton.onClick = [this] { showSaveAsDialog(); };
    settingsButton.setTooltip ("Escala da interface e tema");
    settingsButton.onClick = [this] { showSettingsMenu(); };

    for (auto* c : std::initializer_list<Component*> { &presetBox, &prevButton, &nextButton, &initButton,
                                                       &saveButton, &saveAsButton, &settingsButton })
        addAndMakeVisible (c);
    startTimerHz (30);
}

void HeaderBar::save()
{
    auto& pm = processor.getPresetManager();
    if (pm.getCurrentIndex() >= 0 && ! pm.isCurrentFactory())
        pm.saveUserPreset (pm.getCurrentName(), pm.getCurrentCategory());
    else
        showSaveAsDialog();
}

void HeaderBar::showSaveAsDialog()
{
    auto& pm = processor.getPresetManager();
    auto* w = new AlertWindow ("SALVAR PRESET", "Nome e categoria do novo preset:", MessageBoxIconType::NoIcon);
    w->setLookAndFeel (&getLookAndFeel());
    const auto current = pm.getCurrentName();
    w->addTextEditor ("name", current == "INIT" ? String ("Meu Preset") : current, "NOME");
    w->addComboBox ("cat", PresetManager::categories(), "CATEGORIA");
    if (auto* box = w->getComboBoxComponent ("cat"))
    {
        const int idx = PresetManager::categories().indexOf (pm.getCurrentCategory());
        box->setSelectedItemIndex (idx >= 0 ? idx : 0, dontSendNotification);
    }
    w->addButton ("SALVAR", 1, KeyPress (KeyPress::returnKey));
    w->addButton ("CANCELAR", 0, KeyPress (KeyPress::escapeKey));

    SafePointer<HeaderBar> self (this);
    w->enterModalState (true, ModalCallbackFunction::create ([self, w] (int result)
    {
        if (result != 1 || self == nullptr)
            return;
        const auto name = w->getTextEditorContents ("name").trim();
        const auto cat  = w->getComboBoxComponent ("cat")->getText();
        if (name.isEmpty() || ! self->processor.getPresetManager().saveUserPreset (name, cat))
            AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon, "SALVAR PRESET",
                                              "Nao foi possivel salvar. Verifique o nome e a permissao da pasta Documentos.");
    }), true);
}

void HeaderBar::showSettingsMenu()
{
    PopupMenu scaleMenu, themeMenu;
    const float current = processor.getUiScale();
    for (float s : { 0.8f, 1.0f, 1.25f, 1.5f })
        scaleMenu.addItem (String (roundToInt (s * 100)) + "%", true, std::abs (current - s) < 0.01f,
                           [this, s] { if (onScaleChosen) onScaleChosen (s); });

    for (auto* t : Theme::all())
    {
        const auto name = t->name;
        themeMenu.addItem (name, true, processor.getThemeName() == name,
                           [this, name] { if (onThemeChosen) onThemeChosen (name); });
    }

    PopupMenu menu;
    menu.addSectionHeader ("ATDT METAL MACHINE  v" JucePlugin_VersionString);
    menu.addSubMenu ("UI SCALE", scaleMenu);
    menu.addSubMenu ("THEME", themeMenu);
    menu.addSeparator();
    menu.addItem ("Abrir pasta de presets do usuario", []
    {
        auto f = PresetManager::getUserPresetFolder();
        f.createDirectory();
        f.startAsProcess();
    });
    menu.addItem ("Recarregar lista de presets", [this] { processor.getPresetManager().rescan(); });
    menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&settingsButton));
}

void HeaderBar::timerCallback()
{
    const auto notes = processor.midiActivity.getNoteOnCount();
    midiLed = (notes != lastNotes) ? 1.0f : midiLed * 0.82f;
    lastNotes = notes;
    meterL = jmax (processor.peakLeft.load(),  meterL * 0.86f);
    meterR = jmax (processor.peakRight.load(), meterR * 0.86f);
    repaint();
    presetBox.repaint();
}

// Larguras compartilhadas entre resized() e paint()
namespace HeaderLayout
{
    constexpr int logo = 44 + 80 + 22 + 116, gap = 14;
    constexpr int arrow = 24, box = 190;
    constexpr int buttons = 44 + 48 + 64 + 56 + 3 * 5;
}

void HeaderBar::resized()
{
    using namespace HeaderLayout;
    auto r = getLocalBounds().reduced (16, 15);
    r.removeFromLeft (logo + gap);
    prevButton.setBounds (r.removeFromLeft (arrow));
    r.removeFromLeft (2);
    presetBox.setBounds (r.removeFromLeft (box));
    r.removeFromLeft (2);
    nextButton.setBounds (r.removeFromLeft (arrow));

    auto right = getLocalBounds().reduced (14, 15).removeFromRight (buttons);
    settingsButton.setBounds (right.removeFromRight (56));
    right.removeFromRight (5);
    saveAsButton.setBounds (right.removeFromRight (64));
    right.removeFromRight (5);
    saveButton.setBounds (right.removeFromRight (48));
    right.removeFromRight (5);
    initButton.setBounds (right.removeFromRight (44));
}

void HeaderBar::paint (Graphics& g)
{
    using namespace HeaderLayout;
    const auto& t = themeOf (*this);
    auto h = getLocalBounds();

    g.setGradientFill (ColourGradient (t.panelRaised, 0, 0, t.background, 0, (float) h.getBottom(), false));
    g.fillRect (h);
    Path line;
    line.startNewSubPath (0, (float) h.getBottom() - 1.0f);
    line.lineTo ((float) h.getRight(), (float) h.getBottom() - 1.0f);
    strokeNeon (g, line, t.accent, 1.5f);

    auto r = h.reduced (16, 0);

    // ---- logo ----
    g.setFont (FontOptions (15.0f, Font::bold));
    g.setColour (t.accent);
    g.drawText ("ATDT", r.removeFromLeft (44), Justification::centredLeft);
    g.setFont (FontOptions (23.0f, Font::bold));
    g.setColour (t.text);
    g.drawText ("METAL", r.removeFromLeft (80), Justification::centredLeft);
    g.setColour (t.accent);
    g.drawText ("//", r.removeFromLeft (22), Justification::centredLeft);
    g.setColour (t.text);
    g.drawText ("MACHINE", r.removeFromLeft (116), Justification::centredLeft);
    r.removeFromLeft (gap + 2 * arrow + 4 + box + 16);

    // ---- BPM / SYNC ----
    const bool host = processor.tempo.hostProvidesTempo();
    g.setFont (FontOptions (10.0f));
    g.setColour (t.textDim);
    g.drawText ("BPM", r.removeFromLeft (26), Justification::centredLeft);
    g.setFont (FontOptions (18.0f, Font::bold));
    g.setColour (t.text);
    g.drawText (String (processor.tempo.getBpm(), 1), r.removeFromLeft (56), Justification::centredLeft);
    g.setFont (FontOptions (10.0f, Font::bold));
    g.setColour (host ? t.accent : t.textDim);
    g.drawText (host ? "SYNC HOST" : "NO HOST", r.removeFromLeft (60), Justification::centredLeft);
    g.setColour (processor.tempo.isPlaying() ? t.accent : t.textDim);
    g.drawText (processor.tempo.isPlaying() ? "PLAY" : "STOP", r.removeFromLeft (36), Justification::centredLeft);

    // ---- lado direito (antes dos botoes) ----
    r.removeFromRight (buttons + 10);
    auto meter = r.removeFromRight (80).reduced (0, 21);
    auto drawBar = [&] (Rectangle<int> b, float level)
    {
        g.setColour (t.track);
        g.fillRoundedRectangle (b.toFloat(), 2.0f);
        const float norm = jlimit (0.0f, 1.0f, (Decibels::gainToDecibels (level, -60.0f) + 60.0f) / 60.0f);
        g.setColour (level >= 0.99f ? Colours::white : t.accent);
        g.fillRoundedRectangle (b.toFloat().withWidth ((float) b.getWidth() * norm), 2.0f);
    };
    drawBar (meter.removeFromTop (meter.getHeight() / 2 - 1), meterL);
    meter.removeFromTop (2);
    drawBar (meter, meterR);
    g.setFont (FontOptions (10.0f));
    g.setColour (t.textDim);
    g.drawText ("OUT", r.removeFromRight (26), Justification::centredLeft);
    r.removeFromRight (6);

    g.drawText ("CPU " + String (processor.getCpuLoadPercent(), 1) + "%", r.removeFromRight (62), Justification::centredLeft);
    g.drawText ("VOICES " + String (processor.activeVoices.load()).paddedLeft ('0', 2), r.removeFromRight (64), Justification::centredLeft);

    auto led = r.removeFromRight (48);
    auto dot = led.removeFromLeft (12).withSizeKeepingCentre (9, 9).toFloat();
    g.setColour (t.track);
    g.fillEllipse (dot);
    g.setColour (t.accent.withAlpha (midiLed));
    g.fillEllipse (dot.expanded (midiLed * 2.0f));
    g.setColour (t.textDim);
    g.drawText ("MIDI", led.withTrimmedLeft (4), Justification::centredLeft);
}

// ======================================================================== UPCOMING
UpcomingPage::UpcomingPage (String t, String p, StringArray f)
    : title (std::move (t)), phase (std::move (p)), features (std::move (f)) {}

void UpcomingPage::paint (Graphics& g)
{
    const auto& th = themeOf (*this);
    auto r = getLocalBounds().toFloat();
    g.setColour (th.panel);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (th.edge);
    g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.0f);

    auto area = getLocalBounds().withSizeKeepingCentre (560, 300);
    g.setFont (FontOptions (30.0f, Font::bold));
    g.setColour (th.text);
    g.drawText (title, area.removeFromTop (44), Justification::centredLeft);
    g.setFont (FontOptions (13.0f, Font::bold));
    g.setColour (th.accent);
    g.drawText ("EM DESENVOLVIMENTO  //  " + phase, area.removeFromTop (26), Justification::centredLeft);
    area.removeFromTop (14);
    g.setFont (FontOptions (14.0f));
    for (auto& f : features)
    {
        auto line = area.removeFromTop (26);
        g.setColour (th.accentDark.brighter (0.4f));
        g.fillRect (line.getX(), line.getCentreY() - 1, 10, 2);
        g.setColour (th.textDim);
        g.drawText (f, line.withTrimmedLeft (20), Justification::centredLeft);
    }
}

// ======================================================================== MAIN VIEW
MainView::MainView (MetalMachineAudioProcessor& p)
    : header (p), processor (p), browser (p.getPresetManager()),
      keyboard (p.keyboardState, MidiKeyboardComponent::horizontalKeyboard)
{
    addAndMakeVisible (header);

    pages.push_back (std::make_unique<SynthPage> (p.apvts));
    pages.push_back (std::make_unique<ModPage> (p));
    pages.push_back (std::make_unique<UpcomingPage> ("EFFECTS", "FASE 6",
        StringArray { "6 slots de efeito com ON/OFF", "Distortion, Saturation, Bitcrush, EQ, Compressor",
                      "Chorus, Flanger, Phaser, Delay, Reverb", "Reordenacao por arrastar" }));
    pages.push_back (std::make_unique<UpcomingPage> ("SEQUENCER", "FASE 3",
        StringArray { "16/32 passos sincronizados ao REAPER", "Note, velocity, gate, accent, slide, mute",
                      "Key/Scale com quantizacao", "Presets: Modern Metal, Industrial, Trap Metal..." }));
    pages.push_back (std::make_unique<UpcomingPage> ("DRUMS", "FASE 4",
        StringArray { "Kick, Snare, Clap, Hat, Open Hat, Perc, Tom, Crash, FX", "Level, pan, pitch, decay por canal",
                      "Patterns: Trap, Dubstep, Half Time, Industrial, Metal", "Suporte a samples WAV licenciados" }));
    pages.push_back (std::make_unique<UpcomingPage> ("PERFORMANCE", "FASE 7",
        StringArray { "Preset, BPM, Key, Macros em tela unica", "Sequencia e padrao de bateria com 1 clique",
                      "Abrir -> escolher preset -> tocar" }));

    const StringArray names { "SYNTH", "MOD", "FX", "SEQ", "DRUMS", "PERFORM" };
    for (int i = 0; i < names.size(); ++i)
    {
        auto* b = tabs.add (new TextButton (names[i]));
        b->setClickingTogglesState (false);
        b->onClick = [this, i] { showTab (i); };
        addAndMakeVisible (b);
        addChildComponent (*pages[(size_t) i]);
    }

    // fontes de modulacao arrastaveis (visiveis em qualquer aba)
    for (int src : { (int) ModSource::Lfo1, (int) ModSource::Lfo2, (int) ModSource::Lfo3, (int) ModSource::Lfo4,
                     (int) ModSource::FilterEnv, (int) ModSource::AmpEnv, (int) ModSource::Velocity,
                     (int) ModSource::ModWheel, (int) ModSource::Aftertouch, (int) ModSource::Note,
                     (int) ModSource::Macro1, (int) ModSource::Macro2, (int) ModSource::Macro3, (int) ModSource::Macro4 })
        addAndMakeVisible (sourceChips.add (new ModSourceChip (src)));

    header.onPresetClicked = [this] { setBrowserVisible (! browser.isVisible()); };
    browser.onClose = [this] { setBrowserVisible (false); };
    addChildComponent (browser);

    keyboard.setAvailableRange (24, 108);
    keyboard.setLowestVisibleKey (36);
    keyboard.setKeyWidth (22.0f);
    addAndMakeVisible (keyboard);

    showTab (0);
}

void MainView::setBrowserVisible (bool show)
{
    browser.setVisible (show);
    if (show) browser.toFront (true);
    for (int i = 0; i < (int) pages.size(); ++i)
        pages[(size_t) i]->setVisible (! show && i == currentTab);
}

void MainView::showTab (int index)
{
    browser.setVisible (false);
    currentTab = index;
    for (int i = 0; i < tabs.size(); ++i)
    {
        tabs[i]->setToggleState (i == index, dontSendNotification);
        pages[(size_t) i]->setVisible (i == index);
    }
}

void MainView::paint (Graphics& g)
{
    const auto& t = themeOf (*this);
    g.fillAll (t.background);

    keyboard.setColour (MidiKeyboardComponent::keyDownOverlayColourId, t.accent);
    keyboard.setColour (MidiKeyboardComponent::mouseOverKeyOverlayColourId, t.accent.withAlpha (0.3f));
    keyboard.setColour (MidiKeyboardComponent::shadowColourId, Colours::transparentBlack);
    keyboard.setColour (MidiKeyboardComponent::keySeparatorLineColourId, t.background);
    keyboard.setColour (MidiKeyboardComponent::whiteNoteColourId, Colour (0xffd4d7dc));
    keyboard.setColour (MidiKeyboardComponent::blackNoteColourId, t.panelRaised);
    keyboard.setColour (MidiKeyboardComponent::upDownButtonBackgroundColourId, t.panel);
    keyboard.setColour (MidiKeyboardComponent::upDownButtonArrowColourId, t.accent);

    g.setFont (FontOptions (10.0f, Font::bold));
    g.setColour (t.textDim);
    g.drawText ("MOD SOURCES", chipLabelArea, Justification::centredRight);
}

void MainView::resized()
{
    auto r = getLocalBounds();
    header.setBounds (r.removeFromTop (60));
    r.reduce (10, 8);

    auto tabRow = r.removeFromTop (28);
    for (auto* b : tabs)
    {
        b->setBounds (tabRow.removeFromLeft (88));
        tabRow.removeFromLeft (4);
    }
    for (int i = sourceChips.size(); --i >= 0;)
    {
        sourceChips[i]->setBounds (tabRow.removeFromRight (42).reduced (0, 4));
        tabRow.removeFromRight (3);
    }
    chipLabelArea = tabRow.removeFromRight (70);
    r.removeFromTop (8);

    keyboard.setBounds (r.removeFromBottom (70));
    keyboard.setKeyWidth ((float) keyboard.getWidth() / 50.0f); // C1..C8 inteiro = 50 teclas brancas
    r.removeFromBottom (8);

    for (auto& p : pages)
        p->setBounds (r);
    browser.setBounds (r);
}
} // namespace mm
