/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cone_texture.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lwyl-the <lwyl-the@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/04/18 15:26:29 by rrhaenys          #+#    #+#             */
/*   Updated: 2019/04/22 20:52:07 by lwyl-the         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rt.h"

static void		uv_correct(double *u, double *v, double max_x, double max_y)
{
	(*u) = (*u) / max_x;
	(*v) = (*v) / max_y;
	while (*u >= 1)
		*u = *u - 1.0;
	while (*v >= 1)
		*v = *v - 1.0;
}

static t_vec3	texture_stretching(t_texture *texture,
									t_rt *rt, t_shape *shape, double uv[2])
{
	int				x;
	int				y;
	int				w;
	int				h;

	w = NOISE_WIDTH;
	h = NOISE_HEIGHT;
	if (shape->effect_type == 0)
	{
		w = texture->surface->w;
		h = texture->surface->h;
	}
	x = fabs(uv[0]) * w;
	y = uv[1] * h;
	return (get_texture_color(texture, (int[2]){x, y}, uv, rt));
}

t_vec3			cone_texture2(t_texture *texture, t_shape *shape, t_rt *rt,
								t_cone_texture obj)
{
	double		u;
	double		v;

	u = acos(vec3_dot(&obj.angle, &obj.unit1)) / (2.0 * M_PI);
	if (obj.r.z < 0)
		u = 1.0 - u;
	v = -(obj.r.y / shape->dims.z);
	v = (v + 1) / 2;
	move_texture(&u, &v, (double[2]){shape->t_dims.x, shape->t_dims.y});
	if (shape->t_dims.z != 0)
	{
		if (shape->effect_type == 0)
			uv_correct(&u, &v, (double)texture->surface->w / shape->t_dims.z,
								(double)texture->surface->h / shape->t_dims.z);
		else
			uv_correct(&u, &v, (double)NOISE_WIDTH / shape->t_dims.z,
								(double)NOISE_HEIGHT / shape->t_dims.z);
	}
	else
		uv_correct(&u, &v, 1, 1);
	return (texture_stretching(texture, rt, shape, (double[2]){u, v}));
}

t_vec3			cone_texture(t_texture *texture, t_shape *shape, t_rt *rt)
{
	t_vec3		unit;
	t_vec3		unit1;
	t_vec3		r;
	t_matrix	rotation;
	t_vec3		angle;

	unit = (t_vec3) {0, 1, 0};
	unit1 = (t_vec3) {1, 0, 0};
	rotation = matrix_multiply(z_rotation_matrix(-shape->unit.z),
				matrix_multiply(y_rotation_matrix(shape->unit.y),
									x_rotation_matrix(-shape->unit.x)));
	vector_matrix_multiply(rotation, &unit);
	if (fabs(vec3_dot(&unit, &rt->normal)) >= 1.0 - 1e-10)
		return (sphere_texture(texture, shape, rt));
	vec3_subtract(&rt->source_point, &shape->center, &r);
	vector_matrix_multiply(shape->rotation, &r);
	angle = (t_vec3){r.x, 0, r.z};
	vec3_normalize(&angle, vec3_length(&angle));
	return (cone_texture2(texture, shape, rt,
							(t_cone_texture){angle, unit1, r}));
}
