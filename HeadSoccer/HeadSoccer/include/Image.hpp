#ifndef __image_hpp
#define __image_hpp

#include "common.hpp"
#include "Res.hpp"

#include <atlimage.h>

class Image : public Resource {
public:
	Image( ) : Resource( ), image_( ) {}
	Image( std::string_view key, const std::string& filePath, bool premultipliedAlpha )
		: Resource( key ), image_( ) {
		auto path = fs::path( filePath ).wstring( );	// ANSI(CP949) -> UTF-16, non-ASCII path safe
		image_.Load( path.c_str( ) );

		if ( !image_.IsNull( ) && premultipliedAlpha ) {
			setPremultipliedAlpha( );
		}
	}

	virtual ~Image( ) {}

	void setPremultipliedAlpha( ) {
		unsigned char* pCol = nullptr;
		auto width = image_.GetWidth( );
		auto height = image_.GetHeight( );

		for ( auto y = 0; y < height; ++y ) {
			for ( auto x = 0; x < width; ++x ) {
				pCol = (unsigned char*)image_.GetPixelAddress( x, y );
				auto alpha = pCol[ 3 ];

				if ( alpha < 255 ) {
					pCol[ 0 ] = ( ( pCol[ 0 ] * alpha ) + 127 ) / 255;
					pCol[ 1 ] = ( ( pCol[ 1 ] * alpha ) + 127 ) / 255;
					pCol[ 2 ] = ( ( pCol[ 2 ] * alpha ) + 127 ) / 255;
				}
			}
		}
	}

	void load( const std::string& filePath, bool premultipliedAlpha ) {
		auto path = fs::path( filePath ).wstring( );	// ANSI(CP949) -> UTF-16, non-ASCII path safe
		image_.Load( path.c_str( ) );

		if ( !image_.IsNull( ) && premultipliedAlpha ) {
			setPremultipliedAlpha( );
		}
	}

	void draw( HDC hdc, const PointFloat& objPos ) {
		image_.Draw( hdc,
			static_cast<int>( objPos.x - image_.GetWidth( ) / 2.f ),
			static_cast<int>( objPos.y - image_.GetHeight( ) / 2.f ),
			image_.GetWidth( ), image_.GetHeight( ) );
	}

	void draw(HDC hdc, const PointFloat& objPos, int stat, int offset) {
		int originalWidth = image_.GetWidth();
		int originalHeight = image_.GetHeight();

		// 10칸으로 나누어 한 칸의 폭 계산
		int segmentWidth = originalWidth / 10;

		// statlen이 0 이하 또는 10 초과 시 경계 처리
		int effectiveSegments = (stat <= 0) ? 1 : ((stat > 10) ? 10 : stat);
		int effectiveWidth = effectiveSegments * segmentWidth;
		if (effectiveWidth > originalWidth) effectiveWidth = originalWidth;

		// 소스 영역: 왼쪽에서 effectiveSegments 칸만큼만 잘라냄
		int srcX = 0; // 왼쪽에서 시작
		int srcWidth = effectiveWidth;
		int srcHeight = originalHeight;

		// 대상 크기와 위치 계산
		int drawX = static_cast<int>(objPos.x ) -15;
		int drawY = static_cast<int>(objPos.y - originalHeight / 2.f)+offset;

		// 이미지 그리기 (소스 영역 지정)
		image_.Draw(hdc, drawX, drawY, effectiveWidth, originalHeight,
			srcX, 0, srcWidth, srcHeight);
	}

	void draw( HDC hdc, int xDest, int yDest, int nDestWidth, int nDestHeight,
		int xSrc, int ySrc, int nSrcWidth, int nSrcHeight ) 
	{
		image_.Draw( hdc, xDest, yDest, nDestWidth, nDestHeight,
			xSrc, ySrc, nSrcWidth, nSrcHeight );
	}

	void draw( HDC hdc, int xDest, int yDest, int nDestWidth, int nDestHeight,
		int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, BYTE alpha ) 
	{
		image_.AlphaBlend( hdc, xDest, yDest, nDestWidth, nDestHeight,
			xSrc, ySrc, nSrcWidth, nSrcHeight, alpha );
	}

	std::int32_t width( ) const {
		return image_.GetWidth( );
	}

	std::int32_t height( ) const {
		return image_.GetHeight( );
	}

	HDC getDC( ) const {
		return image_.GetDC( );
	}

	void releaseDC( ) {
		image_.ReleaseDC( );
	}

private:
	CImage image_;
};

#endif	// __image_hpp