#include "../include/TextLabel.hpp"
#include "../include/Camera.hpp"

TextLabel::TextLabel( std::wstring_view text, int fontHeight, COLORREF color )
	: text_( text ), fontHeight_{ fontHeight }, color_{ color } {
	font_ = CreateFontW( fontHeight, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Arial Black" );
}

TextLabel::~TextLabel( ) {
	DeleteObject( font_ );
}

void TextLabel::render( HDC hdc ) {
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

	SetTextColor( hdc, RGB( 0, 0, 0 ) );
	for ( auto dx = -4; dx <= 4; dx += 4 ) {
		for ( auto dy = -4; dy <= 4; dy += 4 ) {
			TextOutW( hdc, x + dx, y + dy, text.data( ), len );
		}
	}

	SetTextColor( hdc, color );
	TextOutW( hdc, x, y, text.data( ), len );
}
