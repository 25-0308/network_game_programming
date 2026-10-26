#include "../include/CollisionHandler.hpp"
#include "../include/SceneHandler.hpp"

void CollisionHandler::update( ) {
	for ( auto row = 0; row < etoi( ObjectType::EOE ); ++row ) {
		for ( auto col = row; col < etoi( ObjectType::EOE ); ++col ) {
			if ( collTable_[ row ] & ( 1 << col ) ) {
				collUpdate(
					static_cast<ObjectType>( row ),
					static_cast<ObjectType>( col )
				);
			}
		}
	}
}

void CollisionHandler::checkCollision( ObjectType lhs, ObjectType rhs ) {
	// 더 작은 값의 그룹 타입을 행으로,
	// 더 큰 값의 그룹 타입을 열(비트)로 사용한다.
	auto row = std::min( etoi( lhs ), etoi( rhs ) );
	auto col = std::max( etoi( lhs ), etoi( rhs ) );

	// 충돌 검사를 위한 비트를 켠다.
	if ( collTable_[ row ] & ( 1 << col ) ) {
		collTable_[ row ] &= ~( 1 << col );
	}
	else {
		collTable_[ row ] |= 1 << col;
	}
}

void CollisionHandler::collUpdate( ObjectType lhs, ObjectType rhs ) {
	auto currScene = SceneHandler::getCurrScene( );

	const auto& lhsGroup = currScene->getGroup( lhs );
	const auto& rhsGroup = currScene->getGroup( rhs );

	for ( Object* lhsObj : lhsGroup ) {
		if ( lhsObj == nullptr || lhsObj->getCollider( ) == nullptr ) {
			continue;
		}

		for ( Object* rhsObj : rhsGroup ) {
			if ( rhsObj == nullptr || rhsObj->getCollider( ) == nullptr || lhsObj == rhsObj ) {
				continue;
			}

			auto combi = ColliderID{
				.lr = {
					.left = lhsObj->getCollider( )->id( ),
					.right = rhsObj->getCollider( )->id( )
				}
			};

			auto iter = collInfo_.find( combi.id );

			if ( iter == collInfo_.end( ) ) {
				collInfo_.insert( { combi.id, false } );
				iter = collInfo_.find( combi.id );
			}

			if ( isCollided( lhsObj->getCollider( ), rhsObj->getCollider( ) ) ) {
				if ( iter->second ) {
					if ( !lhsObj->isAlive( ) || !rhsObj->isAlive( ) ) {
						lhsObj->onCollisionExit( rhsObj );
						rhsObj->onCollisionExit( lhsObj );
						iter->second = false;
					}
					else {
						lhsObj->onCollision( rhsObj );
						rhsObj->onCollision( lhsObj );
					}
				}
				else {
					if ( lhsObj->isAlive( ) && rhsObj->isAlive( ) ) {
						lhsObj->onCollisionEntry( rhsObj );
						rhsObj->onCollisionEntry( lhsObj );
						iter->second = true;
					}
				}
			}
			else {
				if ( iter->second ) {
					lhsObj->onCollisionExit( rhsObj );
					rhsObj->onCollisionExit( lhsObj );
					iter->second = false;
				}
			}
		}
	}
}

bool CollisionHandler::isCollided( Collider* lhs, Collider* rhs ) {
	auto lPos = lhs->getFinalPos( );
	auto lScale = lhs->getScale( );

	auto rPos = rhs->getFinalPos( );
	auto rScale = rhs->getScale( );

	if ( abs( lPos.x - rPos.x ) < ( lScale.x + rScale.x ) / 2.f
		&& abs( lPos.y - rPos.y ) < ( lScale.y + rScale.y ) / 2.f ) {
		return true;
	}

	return false;
}

std::array<std::uint32_t, etoi( ObjectType::EOE )> CollisionHandler::collTable_;
std::map<std::uint64_t, bool> CollisionHandler::collInfo_;
