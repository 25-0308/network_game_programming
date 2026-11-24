#include "../include/TextLabel.hpp"
#include "../include/Camera.hpp"

TextLabel::TextLabel( std::wstring_view text, int fontHeight, COLORREF color )
	: text_( text ), fontHeight_{ fontHeight }, color_{ color } {
	const bool smallLabel = fontHeight <= 32;
	font_ = CreateFontW( smallLabel ? -fontHeight : fontHeight, 0, 0, 0,
		smallLabel ? FW_BOLD : FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH,
		smallLabel ? L"Arial" : L"Arial Black" );
}

TextLabel::~TextLabel( ) {
	DeleteObject( font_ );
}

void TextLabel::render( HDC hdc ) {
	if ( text_.empty( ) ) {
		return;
	}
	auto relativePos = pos_ - Camera::getPos( );

	auto oldFont = (HFONT)SelectObject( hdc, font_ );
	auto oldMode = SetBkMode( hdc, TRANSPARENT );
	auto oldAlign = SetTextAlign( hdc, TA_CENTER | TA_TOP );

	drawOutlined( hdc, text_,
		static_cast<int>( relativePos.x ),
		static_cast<int>( relativePos.y - fontHeight_ / 2.f ),
		color_ );

	SetTextAlign( hdc, oldAlign );
	SetBkMode( hdc, oldMode );
	SelectObject( hdc, oldFont );
}

void TextLabel::drawOutlined( HDC hdc, std::wstring_view text, int x, int y, COLORREF color ) {
	auto len = static_cast<int>( text.size( ) );
	auto oldMode = SetBkMode( hdc, TRANSPARENT );
	if ( auto pen = CreatePen( PS_SOLID, 12, RGB( 0, 0, 0 ) ) ) {
		auto oldPen = SelectObject( hdc, pen );
		if ( BeginPath( hdc ) ) {
			if ( TextOutW( hdc, x, y, text.data( ), len ) && EndPath( hdc ) ) {
				StrokePath( hdc );
			}
			else {
				AbortPath( hdc );
			}
		}
		SelectObject( hdc, oldPen );
		DeleteObject( pen );
	}

	SetTextColor( hdc, color );
	TextOutW( hdc, x, y, text.data( ), len );
	SetBkMode( hdc, oldMode );
}
