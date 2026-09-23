#include "MetalLookAndFeel.h"

using namespace juce;

namespace mm
{
MetalLookAndFeel::MetalLookAndFeel() { setTheme (Theme::metal()); }

void MetalLookAndFeel::setTheme (const Theme& t)
{
    theme = &t;
    setColour (Slider::textBoxTextColourId,        t.text);
    setColour (Slider::textBoxOutlineColourId,     Colours::transparentBlack);
    setColour (Slider::textBoxBackgroundColourId,  Colours::transparentBlack);
    setColour (Slider::textBoxHighlightColourId,   t.accent.withAlpha (0.4f));
    setColour (Label::textColourId,                t.textDim);
    setColour (ScrollBar::thumbColourId,           t.accent.withAlpha (0.7f));
    setColour (ScrollBar::trackColourId,           t.track);
    setColour (Slider::trackColourId,              t.accent.withAlpha (0.55f));
    setColour (Slider::backgroundColourId,         t.display);
    setColour (TextEditor::outlineColourId,        t.edge);
    setColour (TextEditor::focusedOutlineColourId, t.accent);
    setColour (ComboBox::backgroundColourId,       t.display);
    setColour (ComboBox::textColourId,             t.text);
    setColour (ComboBox::outlineColourId,          t.edge);
    setColour (ComboBox::arrowColourId,            t.accent);
    setColour (PopupMenu::backgroundColourId,      t.panelRaised);
    setColour (PopupMenu::textColourId,            t.text);
    setColour (PopupMenu::headerTextColourId,      t.accent);
    setColour (PopupMenu::highlightedBackgroundColourId, t.accent);
    setColour (PopupMenu::highlightedTextColourId, t.background);
    setColour (TextButton::buttonColourId,         t.panelRaised);
    setColour (TextButton::buttonOnColourId,       t.accent);
    setColour (TextButton::textColourOffId,        t.text);
    setColour (TextButton::textColourOnId,         t.background);
    setColour (TextEditor::backgroundColourId,     t.display);
    setColour (TextEditor::textColourId,           t.text);
    setColour (TextEditor::highlightColourId,      t.accent.withAlpha (0.4f));
    setColour (CaretComponent::caretColourId,      t.accent);
}

void MetalLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int width, int height,
                                         float pos, float startAngle, float endAngle, Slider& slider)
{
    const auto& t = *theme;
    auto bounds = Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const float radius = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float arcR = radius - 3.0f;
    const bool bipolar = (bool) slider.getProperties().getWithDefault ("bipolar", false);

    Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (t.track);
    g.strokePath (track, PathStrokeType (3.0f, PathStrokeType::curved, PathStrokeType::rounded));

    const float from = bipolar ? (startAngle + endAngle) * 0.5f : startAngle;
    if (std::abs (angle - from) > 0.01f)
    {
        Path value;
        value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, jmin (from, angle), jmax (from, angle), true);
        strokeNeon (g, value, slider.isEnabled() ? t.accent : t.textDim, 2.5f);
    }

    // anel reservado para indicador de modulacao (ativado quando a matriz de modulacao existir)
    const float bodyR = radius - 9.0f;
    if (bodyR > 3.0f)
    {
        g.setGradientFill (ColourGradient (t.panelRaised.brighter (0.25f), centre.x, centre.y - bodyR,
                                           t.background, centre.x, centre.y + bodyR, false));
        g.fillEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
        g.setColour (t.edge.brighter (0.3f));
        g.drawEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

        Path pointer;
        pointer.addRoundedRectangle (-1.25f, -bodyR + 2.5f, 2.5f, bodyR * 0.55f, 1.25f);
        g.setColour (t.text);
        g.fillPath (pointer, AffineTransform::rotation (angle).translated (centre.x, centre.y));
    }
}

void MetalLookAndFeel::drawComboBox (Graphics& g, int width, int height, bool, int, int, int, int, ComboBox& box)
{
    const auto& t = *theme;
    auto r = Rectangle<float> (0, 0, (float) width, (float) height).reduced (0.5f);
    g.setColour (t.display);
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (box.isMouseOver (true) ? t.accent : t.edge.brighter (0.2f));
    g.drawRoundedRectangle (r, 3.0f, 1.0f);

    Path arrow;
    const float ax = (float) width - 11.0f, ay = (float) height * 0.5f;
    arrow.addTriangle (ax - 3.5f, ay - 1.5f, ax + 3.5f, ay - 1.5f, ax, ay + 2.5f);
    g.setColour (t.accent);
    g.fillPath (arrow);
}

void MetalLookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool over, bool down)
{
    const auto& t = *theme;
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();

    g.setColour (on ? t.accent : (down ? t.accentDark : t.panelRaised));
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (on || over ? t.accent : t.edge.brighter (0.2f));
    g.drawRoundedRectangle (r, 3.0f, 1.0f);
}

Font MetalLookAndFeel::getTextButtonFont (TextButton&, int h) { return Font (FontOptions (jmin (12.0f, (float) h * 0.5f), Font::bold)); }
Font MetalLookAndFeel::getComboBoxFont (ComboBox&)             { return Font (FontOptions (12.0f, Font::bold)); }
Font MetalLookAndFeel::getLabelFont (Label& l)                 { return Font (FontOptions ((float) jmin (11, l.getHeight() - 2))); }
Font MetalLookAndFeel::getPopupMenuFont()                      { return Font (FontOptions (13.0f)); }
// Barra linear: se "bipolar", preenche a partir do centro (ex.: AMOUNT da matriz)
void MetalLookAndFeel::drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                         float minSliderPos, float maxSliderPos, Slider::SliderStyle style, Slider& slider)
{
    if (style != Slider::LinearBar)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    const auto& t = getTheme();
    auto r = Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
    g.setColour (t.display);
    g.fillRoundedRectangle (r, 3.0f);

    const bool bipolar = (bool) slider.getProperties().getWithDefault ("bipolar", false);
    const float from = bipolar ? r.getCentreX() : r.getX();
    auto fill = Rectangle<float>::leftTopRightBottom (jmin (from, sliderPos), r.getY() + 2.0f,
                                                      jmax (from, sliderPos), r.getBottom() - 2.0f);
    g.setColour (t.accent.withAlpha (0.55f));
    g.fillRect (fill);
    if (bipolar)
    {
        g.setColour (t.textDim.withAlpha (0.6f));
        g.drawVerticalLine (roundToInt (r.getCentreX()), r.getY() + 2.0f, r.getBottom() - 2.0f);
    }
    g.setColour (t.edge);
    g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);
}
} // namespace mm
