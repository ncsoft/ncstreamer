/*
  js Update : 2017-3-24 17:43:18
*/
/*
 js Update : 2017-3-23 10:48:26
 */
$(document).ready(function(){

	var event = new CustomEvent("customSelectChange");

// custom select
	$(document).on('click', '[data-ui="custom-select"] .opener', function(e){
		$(this).toggleClass('active');
	});

	$(document).on('click', '[data-ui="custom-select"] li', function(e){
		$(this).parent().prev().removeClass('active').text($(this).text());
		$(this).parent().next().val($(this).attr('data-value'));
		if(!($(this).hasClass('selected'))){
			$(this).addClass('selected').siblings().removeClass('selected');
			document.querySelector('#'+ $(this).closest('[data-ui="custom-select"]').attr('id')).dispatchEvent(event);
		}
	});

	$(document).on('click', function(e){
		if(!($(e.target).is('[data-ui="custom-select"] .opener') || $(e.target).is('[data-ui="custom-select"] li'))){
			$('[data-ui="custom-select"] .opener').removeClass('active');
		}
	});

// tooltip
	$(document).on('click', '.tooltip .layer-close', function(e){
		$('.tooltip').hide();
	});
});
