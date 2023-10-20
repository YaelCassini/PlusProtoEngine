// Author: Peiyao Li
// Date:   Mar 1 2023

#include "Material.hpp"
//求旋转矩阵(两个坐标系)适用于已知转换前的坐标系的三个轴分量，和目标坐标系的一个轴分量。若已知6个坐标轴分量，则注释y,x那两行


//glm::mat4& CoordinateRotation(glm::vec3& u, glm::vec3& v, glm::vec3& w,
//	glm::vec3& x, glm::vec3& y, glm::vec3& z)
//{
//	glm::mat4 transition;
//	// Eigen::Matrix3d transition;
//
//	x = glm::normalize(x);
//	y = glm::normalize(y);
//	z = glm::normalize(z);
//	transition = glm::mat4(
//		glm::vec4(glm::dot(u, x), glm::dot(u, y), glm::dot(u, z), 1.0),
//		glm::vec4(glm::dot(v, x), glm::dot(v, y), glm::dot(v, z), 1.0),
//		glm::vec4(glm::dot(w, x), glm::dot(w, y), glm::dot(w, z), 1.0),
//		glm::vec4(1.0, 1.0, 1.0, 1.0)
//	);
//	return transition;
//}

glm::mat3& CoordinateRotation(const glm::vec3 u, const glm::vec3 v, const glm::vec3 w,
	const glm::vec3 x, const glm::vec3 y, const glm::vec3 z)
{
	glm::mat3 transition;
	// Eigen::Matrix3d transition;

	glm::vec3 _x = glm::normalize(x);
	glm::vec3 _y = glm::normalize(y);
	glm::vec3 _z = glm::normalize(z);
	transition = glm::mat3(
		glm::vec3(glm::dot(u, _x), glm::dot(u, _y), glm::dot(u, _z)),
		glm::vec3(glm::dot(v, _x), glm::dot(v, _y), glm::dot(v, _z)),
		glm::vec3(glm::dot(w, _x), glm::dot(w, _y), glm::dot(w, _z))
	);
	return transition;
}

glm::vec3 HemisphereRotate(const glm::vec3& N, const glm::vec3 dir)
{
	glm::vec3 w = N, u, v;
	do {
		v = glm::vec3(random_range(-1, 1), random_range(-1, 1), random_range(-1, 1));
	} while (glm::length(glm::cross(v, N)) < kEps);
	//if (fabs(w[0]) > 0.1) {
	//	v = glm::vec3(0, 1, 0);
	//}
	//else {
	//	v = glm::vec3(1, 0, 0);
	//}
	v = glm::normalize(glm::cross(v, w));
	u = glm::normalize(glm::cross(v, w));
	

	glm::vec3 res;
	res = dir[0] * u + dir[1] * v + dir[2] * w;
	return glm::normalize(res);

	//glm::mat3 matrix = CoordinateRotation(u, v, w,
	//	glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	//return matrix * dir;
}


//glm::vec3 HemisphereRotate(const glm::vec3& axisz, const glm::vec3 p_local)
//{
//	// Tranform the local coordinate to the world
//	glm::vec3 axisy;
//	do {
//		axisy = glm::vec3(random_range(-1, 1), random_range(-1, 1), random_range(-1, 1));
//	} while (glm::length(glm::cross(axisy, axisz)) < kEps);
//	axisy = glm::normalize(glm::cross(axisy, axisz));
//	glm::vec3 axisx = glm::normalize(glm::cross(axisy, axisz));
//
//	glm::mat3 trans_mat = glm::mat3(axisx, axisy, axisz);
//	glm::vec3 p_world = trans_mat * p_local;
//	return p_world;
//}


 
glm::vec3 HemisphereSample(const glm::vec3& N) {
	glm::vec3 w = N, u, v;
	if (fabs(w[0]) > 0.1) {
		u = glm::vec3(0, 1, 0);
	}
	else {
		u = glm::vec3(1, 0, 0);
	}
	u = glm::normalize(glm::cross(u, w));
	v = glm::normalize(glm::cross(w, u));
	flt r1, r2, r2s;
	r1 = random_float();
	r2 = random_float();
	r1 = r1 * 2 * pi;
	r2s = sqrt(r2);
	glm::vec3 ans = glm::normalize(u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2));
	//std::cout << ans << std::endl;
	return ans;
}

flt PhongMaterial::scatterUniform(glm::vec3& wi, const glm::vec3 normal, const glm::vec3 wo)
{
	wi = HemisphereSample(normal);
	return 1.0 / 2.0 / pi;
}

flt PhongMaterial::scatterLambertian(glm::vec3& wi, const glm::vec3 normal)
{
	flt cos_theta = sqrtf(random_float());
	flt cos_phi = glm::cos(2 * pi * random_float());

	wi = spherical_to_cartesian_cos(cos_theta, cos_phi);

	wi = HemisphereRotate(normal, wi);
	flt pdf = pdfLambertian(wi, normal);

	return pdf;
}

flt PhongMaterial::scatterSpecular(glm::vec3& wi, const glm::vec3 normal, const glm::vec3 wo)
{
	glm::vec3 refle = reflect(-wo, normal);
	flt cos_theta = pow(random_float(),(1.0/(_ns+1)));
	flt cos_phi = glm::cos(2 * pi * random_float());

	wi = spherical_to_cartesian_cos(cos_theta, cos_phi);
	wi = HemisphereRotate(refle, wi);
	flt pdf = pdfSpecular(wi, normal, wo);

	return pdf;
}

flt PhongMaterial::pdfLambertian(const glm::vec3 wi, const glm::vec3 normal)
{
	flt cos = glm::max(flt(0.0), glm::dot(wi, normal));
	return cos / pi;
}

flt PhongMaterial::pdfSpecular(const glm::vec3 wi, const glm::vec3 normal, const glm::vec3 wo)
{
	flt cos = glm::max(flt(0.0), glm::dot(reflect(-wi, normal), wo));
	flt res = (_ns + 1) * flt(0.5) / pi * glm::pow(cos, _ns);
	return res;
}

flt PhongMaterial::scatter(Ray& ray, HitRecord& rec, Ray& scattered)
{
	scattered.setOrigin(rec._pos);
	glm::vec3 wi;
	glm::vec3 wo = -ray.getDirection();

	//flt pdf = scatterUniform(wi, rec.normal, ray.getDirection());
	//scattered.setDirection(wi);
	//return pdf;

	flt ks_weight = log10(_ns);

	flt weight = glm::compMax(_kd) / (glm::compMax(_kd) + ks_weight);

	flt ran = random_float();
	flt pdf_lambertian = 0;
	flt pdf_specular = 0;
	if (ran < weight)
	{
		pdf_lambertian = scatterLambertian(wi, rec._normal);
		pdf_specular = pdfSpecular(wi, rec._normal, -ray.getDirection());
		scattered.setDirection(wi);
	}
	else
	{
		
		pdf_specular = scatterSpecular(wi, rec._normal, wo);
		pdf_lambertian = pdfLambertian(wi, rec._normal);
		scattered.setDirection(wi);
	}

	return weight * pdf_lambertian + (1 - weight) * pdf_specular;
}

glm::vec3 PhongMaterial::bsdf(glm::vec3& wi, HitRecord& rec, glm::vec3& wo)
{
	//return rec.mat->kd;
	glm::vec3 kd = rec._mat->_kd;
	
	if (rec._mat->_has_texture)
	{
		kd = rec._mat->_texture.at(rec._uv);
	}
	glm::vec3 ks = rec._mat->_ks;
	flt ns = rec._mat->_ns;
	glm::vec3 wr = reflect(glm::normalize(-wi), rec._normal);
	flt cos = glm::dot(glm::normalize(wr), wo);
	//glm::vec3 res = kd + ks * (ns + 2) * glm::pow(cos, ns) * flt(0.5);
	glm::vec3 half = glm::normalize(wi + wo);
	glm::vec3 res = kd + 0.125f * ks * (ns + 2) * powf(glm::dot(half, rec._normal), ns);
	res /= pi;
	return res;
}

flt PhongMaterial::pdf(const glm::vec3 wi, const HitRecord rec, const glm::vec3 wo)
{
	flt pdfKd = pdfLambertian(wi, rec._normal);
	flt pdfKs = pdfSpecular(wi, rec._normal, wo);

	flt maxKd = glm::compMax(rec._mat->_kd);
	flt maxKs = glm::compMax(rec._mat->_ks);
	flt ks_weight = log10(_ns);
	flt weightKd = maxKd / (maxKd + ks_weight);

	flt res = weightKd * pdfKd + (1 - weightKd) * pdfKs;
	return res;
}

flt fresnelSchlick(const flt ior1, const flt ior2, const flt cos)
{
	flt f0 = powf((ior1 - ior2) / (ior1 + ior2), 2);
	flt fresnel = f0 + (1 - f0) * powf(1 - cos, 5);
	return fresnel;
}

// ray -> rec -> scattered
flt GlassMaterial::scatter(Ray& ray, HitRecord& rec, Ray& scattered)
{	
	scattered.setOrigin(rec._pos);	
	flt cos_theta = glm::dot(ray.getDirection(), rec._normal);

	// ray enter glass
	if (cos_theta < 0)
	{
		flt fresnel = fresnelSchlick(1.0, rec._mat->_ni, fabs(cos_theta));

		flt ran = random_float();
		if (ran < fresnel)
		{
			glm::vec3 refle = reflect(ray.getDirection(), rec._normal);
			scattered.setDirection(refle);
			return fresnel;
		}
		else
		{
			glm::vec3 refra = refract(ray.getDirection(), rec._normal, 1.0, rec._mat->_ni);
			scattered.setDirection(refra);
			return (1 - fresnel);
		}
	}
	// ray left glass
	else
	{
		flt fresnel = fresnelSchlick(rec._mat->_ni, 1.0, fabs(cos_theta));
		flt ran = random_float();
		//if (ran < fresnel)
		//{
		//	glm::vec3 refle = reflect(ray.getDirection(), rec._normal);
		//	scattered.setDirection(refle);
		//	return fresnel;
		//}
		//else
		//{
			glm::vec3 refra = refract(ray.getDirection(), rec._normal, rec._mat->_ni, 1.0);
			scattered.setDirection(refra);
			return 1 - fresnel;
		//}
	}
}

glm::vec3 GlassMaterial::bsdf(glm::vec3& wi, HitRecord& rec, glm::vec3& wo)
{
	flt flag = glm::dot(wi, rec._normal) * glm::dot(wo, rec._normal);
	if (flag > 0)
	{
		return glm::vec3(1.0);
	}
	else
	{
		return rec._mat->_tr;
	}
}

flt GlassMaterial::pdf(const glm::vec3 wi, const HitRecord rec, const glm::vec3 wo)
{
	return 1.0;
}
