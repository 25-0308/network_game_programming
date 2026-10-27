#ifndef __text_label_hpp
#define __text_label_hpp

#include "Object.hpp"

// Outlined GDI text centered on the object's position, for labels that have no image asset.
class TextLabel : public Object {
public:
	TextLabel( std::wstring_view text, int fontHeight, COLORREF color );
	virtual ~TextLabel( );

	TextLabel( const TextLabel& ) = delete;

	void render( HDC hdc ) override;

	// draws text with a black outline; uses the font and text align already selected in hdc
	static void drawOutlined( HDC hdc, std::wstring_view text, int x, int y, COLORREF color );

private:
	std::wstring text_;
	int fontHeight_;
	COLORREF color_;
	HFONT font_;
};

#endif	// __text_label_hpp
