#pragma once

namespace SurvivalFPSMathFunctions
{
	FORCEINLINE float SmoothIntersection(float Val1, float Val2, float K)
	{
		float H = FMath::Clamp(0.5 - 0.5 * (Val2 - Val1) / K, 0.0, 1.0);
		return FMath::Lerp(Val2, Val1, H) + K * H * (1.0 - H);
	}
};
